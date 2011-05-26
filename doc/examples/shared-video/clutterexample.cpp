#include <clutter/clutter.h>
#include <stdlib.h>

#define UNUSED(x) ((void) (x))

static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event, gpointer data)
{
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        case CLUTTER_space:
            // pass
            break;
        default:
            break;
    }
}

static void on_frame_cb(ClutterTimeline *timeline, guint *ms, gpointer data)
{
    ClutterTexture *texture = CLUTTER_TEXTURE(data);
    UNUSED(texture);
    UNUSED(timeline);
    UNUSED(ms);
    g_print("on_frame_cb\n");
}


int main(int argc, char *argv[])
{
    clutter_init(&argc, &argv);

    ClutterColor black = { 0x00, 0x00, 0x00, 0xff };
    //ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
    ClutterActor *stage = NULL;
    ClutterActor *texture = NULL;
    ClutterTimeline *timeline = NULL;

    /* Get the stage and set its size and color: */
    stage = clutter_stage_get_default();
    clutter_actor_set_size(stage, 800, 600);
    clutter_stage_set_color(CLUTTER_STAGE(stage), &black);

    // Create and add texture actor
    texture = clutter_texture_new();
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), texture);

    // timeline to attach a callback for each frame that is rendered
    timeline = clutter_timeline_new(1000); // ms
    clutter_timeline_set_loop(timeline, TRUE);
    clutter_timeline_start(timeline);
    
    clutter_actor_show_all(stage);

    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_frame_cb), texture);
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb), NULL);

    g_print("Starting the main loop...\n");
    /* Start the main loop, so we can respond to events: */
    clutter_main();

    return EXIT_SUCCESS;
}

