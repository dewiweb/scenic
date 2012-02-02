#include <gst/gst.h>
#include <string>
#include "shared-video-writer.h"

namespace ScenicSharedVideo 
{ 
    Writer::Writer ()
    {}
    
    Writer::Writer (GstElement *pipeline,GstElement *videoElement,const std::string socketPath) : timereset_ (FALSE), timeshift_ (0)
    {
	
	qserial_     = gst_element_factory_make ("queue", NULL);
	serializer_  = gst_element_factory_make ("gdppay",  NULL);
	shmsink_     = gst_element_factory_make ("shmsink", NULL);
    
	if ( !qserial_ || !serializer_ || !shmsink_) {
	    g_printerr ("Writer: One gstreamer element could not be created.\n");
	}

	GstState current;
	gst_element_get_state (pipeline,&current,NULL,GST_CLOCK_TIME_NONE);
	
	g_object_set (G_OBJECT (shmsink_), "socket-path", socketPath.c_str(), NULL);
	g_object_set (G_OBJECT (shmsink_), "shm-size", 94967295, NULL);
	g_object_set (G_OBJECT (shmsink_), "sync", FALSE, NULL);
	g_object_set (G_OBJECT (shmsink_), "wait-for-connection", FALSE, NULL);
    
	//adding a probe for reseting timestamp when reconnecting
	GstPad *qserialPad = gst_element_get_pad (qserial_, "src");
	gst_pad_add_data_probe (qserialPad, 
				G_CALLBACK (Writer::reset_time),
				static_cast<void *>(this));
	gst_object_unref(qserialPad);
	
	g_signal_connect (shmsink_, "client-connected", 
			  G_CALLBACK (Writer::on_client_connected), 
			  static_cast<void *>(this));

	gst_bin_add_many (GST_BIN (pipeline), qserial_, serializer_, shmsink_, NULL);
	gst_element_link_many (videoElement, qserial_, serializer_, shmsink_,NULL);

	if(current != GST_STATE_NULL)
	{
	    gst_element_set_state (qserial_, current);
	    gst_element_set_state (serializer_, current);
	    gst_element_set_state (shmsink_, current);
	}


    }


    Writer::~Writer (){
    }

    gboolean
    Writer::reset_time (GstPad * pad, GstMiniObject * mini_obj, gpointer user_data)
    {
	Writer *context = static_cast<Writer*>(user_data);
	if (GST_IS_EVENT (mini_obj)) {
	}
	else if (GST_IS_BUFFER (mini_obj)) {
	    GstBuffer *buffer = GST_BUFFER_CAST (mini_obj);
	    if(context->timereset_)
	    {
		context->timeshift_=GST_BUFFER_TIMESTAMP(buffer);
		context->timereset_=FALSE;
	    }
	    GST_BUFFER_TIMESTAMP(buffer)=GST_BUFFER_TIMESTAMP(buffer) - context->timeshift_;
	} 
	else if (GST_IS_MESSAGE (mini_obj)) {
	}
    
	return TRUE;
    
    }

    void
    Writer::pad_unblocked (GstPad * pad, gboolean blocked, gpointer user_data)
    {
	Writer *context = static_cast<Writer*>(user_data);

	if(blocked)
	    g_printerr ("Error: pad not unblocked\n");
	else
	{
	    context->timereset_=TRUE;
	}
    }

    void 
    Writer::switch_to_new_serializer (GstPad * pad, gboolean blocked, gpointer user_data )
    {
	Writer *context = static_cast<Writer*>(user_data);

	//unlink the old serializer
	GstPad *srcPad=gst_element_get_static_pad(context->serializer_,"src");
	GstPad *srcPadPeer=gst_pad_get_peer(srcPad);
	if (!gst_pad_unlink (srcPad,srcPadPeer))
	    g_printerr("Error: cannot unlink src\n");

	GstPad *sinkPad=gst_element_get_static_pad(context->serializer_,"sink");
	GstPad *sinkPadPeer=gst_pad_get_peer(sinkPad);
	if (!gst_pad_unlink (sinkPadPeer,sinkPad))
	    g_printerr("Error: cannot unlink sink\n");

	gst_object_unref (srcPad);
	gst_object_unref (sinkPad);
    
	GstBin *bin=GST_BIN (GST_ELEMENT_PARENT(context->serializer_));
	

	//supposed to be PLAYING, possible issue because of not taking care of pending state 
	GstState current;
	gst_element_get_state (context->serializer_,&current,NULL,GST_CLOCK_TIME_NONE);
	//g_print("serializer state: %d \n",current);

	//get rid of the old serializer TODO ensure object has been cleaned up
	gst_element_set_state (context->serializer_,GST_STATE_NULL);

	//waiting for possible async state change
	gst_element_get_state (context->serializer_,NULL,NULL,GST_CLOCK_TIME_NONE);

	//creating and linking the new serializer
	context->serializer_ = gst_element_factory_make ("gdppay",  NULL); 
	if(gst_element_set_state (context->serializer_, current) != GST_STATE_CHANGE_SUCCESS) 
	    g_printerr ("Error: issue changing newSerializer state\n"); 
	else{ 
	    //g_print ("changing serializer state\n"); 
	    gst_bin_add (bin,context->serializer_); 
	    GstPad *newSinkPad=gst_element_get_static_pad (context->serializer_,"sink"); 
	    GstPad *newSrcPad=gst_element_get_static_pad (context->serializer_,"src"); 
	    gst_pad_link (newSrcPad,srcPadPeer); 
	    gst_pad_link (sinkPadPeer,newSinkPad); 
	    gst_object_unref (newSinkPad); 
	    gst_object_unref (newSrcPad); 
	}	 
	gst_object_unref (srcPadPeer); 
	gst_object_unref (sinkPadPeer); 

	//unblocking data stream 
	//g_print ("setting pad to unblocked\n"); 
	gst_pad_set_blocked_async (pad, 
				   FALSE,
				   (GstPadBlockCallback) (Writer::pad_unblocked),
				   static_cast<void *>(context)); 
    }

    void 
    Writer::on_client_connected (GstElement * shmsink, gint num, gpointer user_data) 
    { 
	Writer *context = static_cast<Writer*>(user_data);
	
	//g_print ("on client connected %d\n",num); 
	GstPad *serializerSinkPad=gst_element_get_static_pad(context->serializer_,"sink");
	GstPad *padToBlock = gst_pad_get_peer(serializerSinkPad);
	
	if(!gst_pad_set_blocked_async (padToBlock, 
				       TRUE, 
				       (GstPadBlockCallback) (Writer::switch_to_new_serializer), 
				       static_cast<void *>(context)))
	    g_printerr("Error: requesting the pad to be blocked\n");
	gst_object_unref (serializerSinkPad);
	gst_object_unref (padToBlock);
    } 

} //namespace ScenicSharedVideo 
