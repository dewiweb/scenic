#include <gst/gst.h>

static gboolean bus_call (GstBus *bus, GstMessage *msg, gpointer data)
{
    GMainLoop *loop = data;

    switch (GST_MESSAGE_TYPE (msg)) 
    {
        case GST_MESSAGE_EOS:
            g_print ("End-of-stream\n");
            g_main_loop_quit (loop);
            break;
        case GST_MESSAGE_ERROR: 
            {
                gchar *debug = NULL;
                GError *err = NULL;

                gst_message_parse_error (msg, &err, &debug);

                g_print ("Error: %s\n", err->message);
                g_error_free (err);

                if (debug) {
                    g_print ("Debug deails: %s\n", debug);
                    g_free (debug);
                }

                g_main_loop_quit (loop);
                break;
            }
        default:
            break;
    }

    return TRUE;
}

gint main (gint argc, gchar *argv[])
{
    GstStateChangeReturn ret;
    GstElement *pipeline, *src, *sink;
    GMainLoop *loop;
    GstBus *bus;

    /* initialization */
    gst_init (&argc, &argv);
    loop = g_main_loop_new (NULL, FALSE);

    /* create elements */
    pipeline = gst_pipeline_new ("my_pipeline");

    /* watch for messages on the pipeline's bus (note that this will only
     * work like this when a GLib main loop is running) */
    bus = gst_pipeline_get_bus (GST_PIPELINE (pipeline));
    gst_bus_add_watch (bus, bus_call, loop);
    gst_object_unref (bus);

    src = gst_element_factory_make ("jackaudiosrc", NULL);

    /* putting an audioconvert element here to convert the output of the
     * decoder into a format that my_filter can handle (we are assuming it
     * will handle any sample rate here though) */
    //convert1 = gst_element_factory_make ("audioconvert", "audioconvert1");

    /* use "identity" here for a filter that does nothing */
    //filter   = gst_element_factory_make ("my_filter", "my_filter");

    /* there should always be audioconvert and audioresample elements before
     * the audio sink, since the capabilities of the audio sink usually vary
     * depending on the environment (output used, sound card, driver etc.) */
    //convert2 = gst_element_factory_make ("audioconvert", "audioconvert2");
    //resample = gst_element_factory_make ("audioresample", "audioresample");
    sink = gst_element_factory_make("jackaudiosink", NULL);

    if (!sink)
        g_print ("output could not be found - check your install\n");

    g_object_set(G_OBJECT (src), "connect", 0, NULL);
    g_object_set(G_OBJECT (sink), "connect", 0, NULL);
    g_object_set(G_OBJECT (sink), "sync", FALSE, NULL);

    gst_bin_add_many (GST_BIN (pipeline), src, sink, NULL);

    /* link everything together */
    if (!gst_element_link(src, sink)) {
        g_print ("Failed to link one or more elements!\n");
        return -1;
    }

    /* run */
    ret = gst_element_set_state (pipeline, GST_STATE_PLAYING);
    if (ret == GST_STATE_CHANGE_FAILURE) {
        g_print ("Failed to start up pipeline!\n");

        /* check if there is an error message with details on the bus */
        GstMessage *msg = gst_bus_poll (bus, GST_MESSAGE_ERROR, 0);
        if (msg) {
            GError *err = NULL;

            gst_message_parse_error (msg, &err, NULL);
            g_print ("ERROR: %s\n", err->message);
            g_error_free (err);
            gst_message_unref (msg);
        }
        return -1;
    }

    g_main_loop_run (loop);

    /* clean up */
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);

    return 0;
}
