//
//gcc -Wall  shared-video-writer.c -o shared-video-writer $(pkg-config --cflags --libs gstreamer-0.10)
//

#include <gst/gst.h>
#include <glib.h>


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


int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;

  GstElement *pipeline, *source, *serializer, *shmsink;
  GstBus *bus;

  /* Initialisation */
  gst_init (&argc, &argv);

  loop = g_main_loop_new (NULL, FALSE);

  /* Check input arguments */
  if (argc != 2) {
    g_printerr ("Usage: %s <socket-path>\n", argv[0]);
    return -1;
  }

  /* Create gstreamer elements */
  pipeline   = gst_pipeline_new ("shared-video-writer");
  source     = gst_element_factory_make ("videotestsrc",  "video-source");
  serializer = gst_element_factory_make ("gdppay",  "serializer");
  shmsink       = gst_element_factory_make ("shmsink", "shmoutput");

  if (!pipeline || !source || !serializer || !shmsink) {
    g_printerr ("One element could not be created. Exiting.\n");
    return -1;
  }

 /*specifying video format*/
  GstCaps *videocaps;
  videocaps = gst_caps_new_simple ("video/x-raw-yuv",
     "format", GST_TYPE_FOURCC, GST_MAKE_FOURCC ('I', '4', '2', '0'),
     "framerate", GST_TYPE_FRACTION, 30, 1,
     "pixel-aspect-ratio", GST_TYPE_FRACTION, 1, 1,
     "width", G_TYPE_INT, 1920,
     "height", G_TYPE_INT, 1080,
     NULL);

  /* we add a message handler */
  bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
  gst_bus_add_watch (bus, bus_call, loop);
  gst_object_unref (bus);

  /* we add all elements into the pipeline */
  gst_bin_add_many (GST_BIN (pipeline),
                    source, serializer, shmsink, NULL);
 
  /* we link the elements together */
  gst_element_link_filtered (source, serializer,videocaps);
  gst_element_link (serializer, shmsink);

  /* Set up the pipeline */
  g_object_set (G_OBJECT (shmsink), "socket-path", argv[1], NULL);
  g_object_set (G_OBJECT (shmsink), "shm-size", 94967295, NULL);
  g_object_set (G_OBJECT (shmsink), "sync", FALSE, NULL);

  /* Set the pipeline to "playing" state*/
  g_print ("Now writing: %s\n", argv[1]);
  gst_element_set_state (pipeline, GST_STATE_PLAYING);

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
