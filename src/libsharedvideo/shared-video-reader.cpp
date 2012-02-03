#include "shared-video.h"

namespace ScenicSharedVideo 
{ 

    Reader::Reader ()
    {}
    

    Reader::Reader (GstElement *pipeline,GstElement *sink,const std::string socketPath)
    {
	source_       = gst_element_factory_make ("shmsrc",  NULL);
	deserializer_ = gst_element_factory_make ("gdpdepay",  NULL);   
	pipeline_     = pipeline;
	if ( !source_ || !deserializer_ ) {
	    g_printerr ("One element could not be created. Exiting.\n");
	}
	
	g_object_set (G_OBJECT (source_), "socket-path", socketPath.c_str(), NULL);
	
	gst_bin_add_many (GST_BIN (pipeline_),
			  source_, deserializer_, NULL);


	gst_element_link_many (source_, deserializer_,sink, NULL);

	//monitoring the shared memory file
	shmfile_ = g_file_new_for_commandline_arg (socketPath.c_str());
	if (shmfile_ == NULL) {
	    g_printerr ("argument not valid. \n");
	}
	
	if (g_file_query_exists (shmfile_,NULL)){
	    g_print ("Now reading: \n");
	    gst_element_set_state (pipeline_, GST_STATE_PLAYING);
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
		gst_element_set_state (context->pipeline_, GST_STATE_PLAYING);
	    }	  
	    g_print ("G_FILE_MONITOR_EVENT_CREATED: %s\n",filename);
	    break;
	case G_FILE_MONITOR_EVENT_DELETED:
	    if (g_file_equal (file,context->shmfile_)) {
		g_print ("Now nulling: \n");
		gst_element_set_state (context->pipeline_, GST_STATE_NULL);
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
