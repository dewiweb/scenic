#include "shared-video.h"

namespace ScenicSharedVideo 
{ 

    Reader::Reader ()
    {}

    Reader::Reader (GstElement *pipeline,GstElement *sink,const std::string socketName) 	
    {
	pipeline_     = pipeline;
	sink_         = sink;
	socketName_.append(socketName);

	GstStaticPadTemplate video_template =
	   GST_STATIC_PAD_TEMPLATE (
	       "sink",          
	       GST_PAD_SINK,    // the direction of the pad
	       GST_PAD_REQUEST,  // when this pad will be present
	       GST_STATIC_CAPS ("video")
	       );

	tmpl_ = gst_static_pad_template_get (&video_template);

	if(tmpl_ == NULL ) g_print ("fucking template\n");
	else g_print ("joie\n");

	//monitoring the shared memory file
	shmfile_ = g_file_new_for_commandline_arg (socketName_.c_str());
	if (shmfile_ == NULL) {
	    g_printerr ("argument not valid. \n");
	}
	
	if (g_file_query_exists (shmfile_,NULL)){
	    g_print ("Now reading constructor: \n");
	    //gst_element_set_state (pipeline_, GST_STATE_PLAYING);
	    Reader::attach ();
	}
	
	GFile *dir = g_file_get_parent (shmfile_);
	dirMonitor_ = g_file_monitor_directory (dir, G_FILE_MONITOR_NONE, NULL, NULL); 
	g_object_unref(dir);
	g_signal_connect (dirMonitor_, "changed", G_CALLBACK (Reader::file_system_monitor_change), static_cast<void *>(this)); 
    }


    Reader::~Reader (){
	g_object_unref(shmfile_);
	g_object_unref(dirMonitor_);
    }

    void 
    Reader::attach ()
    {
	source_       = gst_element_factory_make ("shmsrc",  NULL);
	deserializer_ = gst_element_factory_make ("gdpdepay",  NULL);   
	if ( !source_ || !deserializer_ ) {
	    g_printerr ("One element could not be created. Exiting.\n");
	}
	g_object_set (G_OBJECT (source_), "socket-path", socketName_.c_str(), NULL);
	
	gst_bin_add_many (GST_BIN (pipeline_),
			  source_, deserializer_, NULL);

	sinkPad_ = gst_element_request_pad (sink_,tmpl_,NULL,NULL);
	deserialPad_ = gst_element_get_static_pad (deserializer_,"src");

	gst_element_link (source_, deserializer_);
	gst_pad_link (deserialPad_,sinkPad_);
	
	gst_element_set_state (deserializer_, GST_STATE_PLAYING);
	gst_element_set_state (source_, GST_STATE_PLAYING);
    }

    void
    Reader::detach ()
    {
	gst_element_set_state (deserializer_, GST_STATE_NULL);
	gst_element_set_state (source_, GST_STATE_NULL);
	gst_element_unlink (source_, deserializer_);
	gst_pad_unlink (deserialPad_,sinkPad_);
	gst_object_unref (deserialPad_);
	gst_element_release_request_pad(sink_,sinkPad_);
	gst_object_unref(sinkPad_);
	gst_bin_remove (GST_BIN (pipeline_),source_); 
	gst_bin_remove (GST_BIN (pipeline_),deserializer_);
	
    }


    void
    Reader::file_system_monitor_change (GFileMonitor *      monitor,
					GFile *             file,
					GFile *             other_file,
					GFileMonitorEvent   type,
					gpointer user_data)
    {
	
	char *filename = g_file_get_path (file);

	Reader *context = static_cast<Reader*>(user_data);

	switch (type)
	{
	case G_FILE_MONITOR_EVENT_CREATED:
	    if (g_file_equal (file,context->shmfile_)) {
		g_print ("Now reading: \n");
		context->attach();
		//gst_element_set_state (context->pipeline_, GST_STATE_PLAYING);
	    }	  
	    g_print ("G_FILE_MONITOR_EVENT_CREATED: %s\n",filename);
	    break;
	case G_FILE_MONITOR_EVENT_DELETED:
	    if (g_file_equal (file,context->shmfile_)) {
		g_print ("Now nulling: \n");
//		gst_element_set_state (context->pipeline_, GST_STATE_NULL);
		context->detach ();
	    }	  
	    g_print ("G_FILE_MONITOR_EVENT_DELETED: %s\n",filename);
	    break;
	    /* case G_FILE_MONITOR_EVENT_CHANGED: */
	    /* 	  g_print ("G_FILE_MONITOR_EVENT_CHANGED\n"); */
	    /* 	  break; */
	    /* case G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED: */
	    /* 	  g_print ("G_FILE_MONITOR_EVENT_ATTRIBUTE_CHANGED\n"); */
	    /* 	  break; */
	    /* case G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT: */
	    /* 	  g_print ("G_FILE_MONITOR_EVENT_CHANGES_DONE_HINT\n"); */
	    /* 	  break; */
	    /* case G_FILE_MONITOR_EVENT_PRE_UNMOUNT: */
	    /* 	  g_print ("G_FILE_MONITOR_EVENT_PRE_UNMOUNT\n"); */
	    /* 	  break; */
	    /* case G_FILE_MONITOR_EVENT_UNMOUNTED: */
	    /* 	  g_print ("G_FILE_MONITOR_EVENT_UNMOUNTED\n"); */
	    /* 	  break; */
	default:
	    break;
	}
	g_free (filename);
	
    }
    

} //end namespace  ScenicSharedVideo
