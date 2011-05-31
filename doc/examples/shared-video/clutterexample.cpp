#include <clutter/clutter.h>
#include <stdlib.h>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/lexical_cast.hpp>

#include "sharedVideoBuffer.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#define UNUSED(x) ((void) (x))

/// FUCKING GROSS!!!
static int GLOBAL_width = 0;
static int GLOBAL_height = 0;
static SharedVideoBuffer *sharedBuffer;

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
    //ClutterTexture *texture = CLUTTER_TEXTURE(data);
    clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(data),sharedBuffer->pixelsAddress(),true,GLOBAL_width,GLOBAL_height,GLOBAL_width*4,4,CLUTTER_TEXTURE_NONE,NULL);
    //UNUSED(texture);
    UNUSED(timeline);
    UNUSED(ms);
    //g_print("on_frame_cb\n");
}

int main(int argc, char *argv[])
{
    using namespace boost::interprocess;

    std::string sharedMemoryId("shared_memory");
    switch (argc)
    {
        case 2: // just id
            sharedMemoryId = std::string(argv[1]);
            break;
        default:
            GLOBAL_width = 640;
            GLOBAL_height = 480;
            break;
    }


    bool opened = false;
    while (!opened)
    {
        try
        {
            // open the already created shared memory object
            shared_memory_object shm(open_only, sharedMemoryId.c_str(), read_write);
            opened = true; 

            // map the whole shared memory in this process
            mapped_region region(shm, read_write);

            // get the address of the region
            void *addr = region.get_address();

            // cast to pointer of type of our shared structure
            sharedBuffer = static_cast<SharedVideoBuffer*>(addr);
            GLOBAL_width = sharedBuffer->getWidth();
            GLOBAL_height = sharedBuffer->getHeight();
            
            std::cout << "resolution = " << GLOBAL_width << "x" << GLOBAL_height
                << std::endl;

            //SharedVideoPlayer player;

            // grab the ptr
            //player.init(sharedBuffer->pixelsAddress());

            // start our consumer thread, which is a member function of our player object and
            // takes sharedBuffer as an argument
            /*
            boost::thread worker(boost::bind<void>(boost::mem_fn(&SharedVideoPlayer::consumeFrame), 
                        boost::ref(player), 
                        sharedBuffer));
            */

            //player.run();

            //player.signalKilled(); // let worker know that the mainloop has exitted
            //worker.join(); // wait for worker to end out before main thread does
            std::cout << "Main thread going out\n";
        }
        catch(interprocess_exception &ex)
        {
            static const char *MISSING_ERROR = "No such file or directory";
            if (strncmp(ex.what(), MISSING_ERROR, strlen(MISSING_ERROR)) != 0)
            {
                shared_memory_object::remove(sharedMemoryId.c_str());
                std::cout << "Unexpected exception: " << ex.what() << std::endl;
                return 1;
            }
            else
            {
                std::cerr << "Shared buffer doesn't exist yet\n";
                //boost::this_thread::sleep(boost::posix_time::milliseconds(30)); 
            }
        }
    }   // end while not opened


    clutter_init(&argc, &argv);

    ClutterColor blue = { 0x00, 0x00, 0xff, 0xff };
    //ClutterColor white = { 0xff, 0xff, 0xff, 0xff };
    ClutterActor *stage = NULL;
    ClutterActor *texture = NULL;
    ClutterTimeline *timeline = NULL;

    /* Get the stage and set its size and color: */
    stage = clutter_stage_get_default();
    clutter_actor_set_size(stage, 640, 480);
    clutter_stage_set_color(CLUTTER_STAGE(stage),&blue); 

    // Create and add texture actor
    texture = clutter_texture_new();
    //clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(texture),sharedBuffer->pixelsAddress(),false,640,480,640*3,3,CLUTTER_TEXTURE_NONE,NULL);
    //texture = clutter_rectangle_new_with_color(&white);
    clutter_actor_set_size(texture, GLOBAL_width, GLOBAL_height);
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

