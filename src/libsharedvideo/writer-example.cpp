#include <gst/gst.h>
#include <signal.h>
#include <string>
#include "shared-video-writer.h"


GstElement *pipeline;

GstElement *source;    
GstElement *tee;       
GstElement *qlocalxv;  
GstElement *imgsink;   
GstElement *timeoverlay;
GstElement *camsource;

const std::string *socketName;
ScenicSharedVideo::Writer *writer;

//clean up pipeline when ctrl-c
void
leave(int sig) {
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));

    exit(sig);
}



static gboolean
bus_call (GstBus     *bus,
          GstMessage *msg,
          gpointer    data)
{
    GMainLoop *loop = (GMainLoop *) data;

    switch (GST_MESSAGE_TYPE (msg)) {

    case GST_MESSAGE_EOS:
	g_print ("End of stream\n");
	g_main_loop_quit (loop);
	break;

    case GST_MESSAGE_ERROR: {
	gchar  *debug;
	GError *error;

	gst_message_parse_error (msg, &error, &debug);
	g_free (debug);

	g_printerr ("Error: %s\n", error->message);
	g_error_free (error);

	g_main_loop_quit (loop);
	break;
    }
    default:
	break;
    }

    return TRUE;
}


// static gboolean  
// add_shared_video_writer
// {
//     // const std::string socketName (argv[1]);
//     // ScenicSharedVideo::Writer writer (pipeline,tee,socketName);
//     return FALSE;
// }

int
main (int   argc,
      char *argv[])
{
    (void) signal(SIGINT,leave);

    /* Initialisation */
    gst_init (&argc, &argv);

    GMainLoop *loop = g_main_loop_new (NULL, FALSE);

    /* Check input arguments */
    if (argc != 2) {
	g_printerr ("Usage: %s <socket-path>\n", argv[0]);
	return -1;
    }

    /* Create gstreamer elements */
    pipeline    = gst_pipeline_new ("shared-video-writer");
    source      = gst_element_factory_make ("videotestsrc",  "video-source");
    
    camsource   = gst_element_factory_make ("v4l2src",  NULL);

    timeoverlay = gst_element_factory_make ("timeoverlay", NULL);
    tee         = gst_element_factory_make ("tee", NULL);

 
    qlocalxv    = gst_element_factory_make ("queue", NULL);
    imgsink     = gst_element_factory_make ("xvimagesink", NULL);

    
    if (!pipeline || !source || !timeoverlay || !tee || !qlocalxv || !imgsink ) {
	g_printerr ("One element could not be created. Exiting.\n");
	return -1;
    }

    /*specifying video format*/
     /* GstCaps *videocaps;  */
     /* videocaps = gst_caps_new_simple ("video/x-raw-yuv",  */
     /* 				     "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),  */
     /* 				     "framerate", GST_TYPE_FRACTION, 30, 1,  */
     /* 				     "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,  */
     /* 				     /\* "width", G_TYPE_INT, 600,  *\/ */
     /* 				     /\* "height", G_TYPE_INT, 400,  *\/ */
     /* 				      "width", G_TYPE_INT, 1920,   */
     /* 				      "height", G_TYPE_INT, 1080,   */
     /* 				     NULL);  */

    /* we add a message handler */
    GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    /* we add all elements into the pipeline */
    gst_bin_add_many (GST_BIN (pipeline),
		      //source, 
		      camsource, 
		      timeoverlay, tee, qlocalxv, imgsink, NULL);

    //init before set playing state 
    // *socketName = std::string (argv[1]);
    // *writer = ScenicSharedVideo::Writer (pipeline,tee,socketName);


    /* we link the elements together */
    //gst_element_link_filtered (source, timeoverlay,videocaps);
    gst_element_link (camsource,timeoverlay);
    gst_element_link (timeoverlay, tee);
    gst_element_link_many (tee, qlocalxv,imgsink,NULL);
        
    g_object_set (G_OBJECT (imgsink), "sync", FALSE, NULL);


    /* Set the pipeline to "playing" state*/
    g_print ("Now writing: %s\n", argv[1]);
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
    

    //g_timeout_add (1000, (GSourceFunc) add_shared_video_writer, NULL);


    /* Iterate */
    g_print ("Running...\n");
    g_main_loop_run (loop);

    /* Out of the main loop, clean up nicely */
    g_print ("Returned, stopping playback\n");
    gst_element_set_state (pipeline, GST_STATE_NULL);

    g_print ("Deleting pipeline\n");
    gst_object_unref (GST_OBJECT (pipeline));

    return 0;
}
