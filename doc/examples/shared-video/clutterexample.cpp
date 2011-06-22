#include <clutter/clutter.h>
#include <stdlib.h>
#include <iostream>

#include <boost/bind.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/lexical_cast.hpp>

#include "./clutterexample.h"
#include "sharedVideoBuffer.h"

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>

#define UNUSED(x) ((void) (x))

//static int __aRBLookUpTable[32] = {5,18,29,38,48,56,64,71,78,85,92,99,106,113,119,125,131,137,144,151,157,164,171,177,184,192,200,208,216,226,238,250};
//static int __aGLookUpTable[64] = {3,10,16,22,27,32,36,41,46,50,54,57,61,65,69,73,77,80,83,86,90,93,96,100,103,106,109,113,116,119,123,126,130,133,136,139,142,145,149,152,155,158,162,165,168,172,175,178,182,185,189,193,196,200,205,209,213,218,222,227,233,239,246,252};

/// FUCKING GROSS!!!
static int GLOBAL_width = 0;
static int GLOBAL_height = 0;
int framecount = 0;

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
    //SharedVideoPlayer *player = (SharedVideoPlayer*)&data;
    boost::mutex::scoped_lock displayLock(((SharedVideoPlayer*)data)->displayMutex_);

    bool feedback = clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(((SharedVideoPlayer*)data)->texture),((SharedVideoPlayer*)data)->pixels,false,GLOBAL_width,GLOBAL_height,0,3,CLUTTER_TEXTURE_NONE,NULL);
    if (!feedback) {
        std::cout << "failed" << std::endl;
    }

    UNUSED(timeline);
    UNUSED(ms);

    framecount++;

    ((SharedVideoPlayer*)data)->textureUploadedCondition_.notify_one();
}

void SharedVideoPlayer::consumeFrame(SharedVideoBuffer *sharedBuffer)
{
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::shared_memory_object;

    std::cout << "\nWorker thread started.\n";

    // get frames until the other process marks the end
    bool end_loop = false;

    // make sure there's no sentinel
    {
        // Lock the mutex
        //scoped_lock<interprocess_mutex> lock(sharedBuffer->getMutex());
        //sharedBuffer->startPushing();   // tell appsink to give us buffers
        boost::mutex::scoped_lock displayLock(displayMutex_);
    }


    do
    {
        {
            // Lock the mutex
            scoped_lock<interprocess_mutex> lock(sharedBuffer->getMutex());

            // wait for new buffer to be pushed if it's empty
            //sharedBuffer->waitOnProducer(lock);

            if (not sharedBuffer->waitOnProducer(lock)) 
            {
                end_loop = true;
            }
            else
            {
                // got a new buffer, wait until we upload it in gl thread before notifying producer
                {
                    boost::mutex::scoped_lock displayLock(displayMutex_);

                    if (killed_)
                    {
                        end_loop = true;
                    }
                    else
                        textureUploadedCondition_.wait(displayLock);
                }

                // Notify the other process that the buffer status has changed
                sharedBuffer->notifyProducer();
            }
            // mutex is released (goes out of scope) here
        }
    }
    while (!end_loop);

    std::cout << "\nWorker thread Going out.\n";
    // erase shared memory
    // shared_memory_object::remove("shared_memory");
}

SharedVideoPlayer::SharedVideoPlayer() : 
    displayMutex_(), textureUploadedCondition_(), killed_(false)
{}

void SharedVideoPlayer::init(unsigned char *pixelData) {

    pixels = pixelData;
    ClutterColor blue = { 0x00, 0x00, 0xff, 0xff };

    /* Get the stage and set its size and color: */
    stage = clutter_stage_get_default();
    //clutter_actor_set_size(stage, GLOBAL_width, GLOBAL_height);
    clutter_stage_set_color(CLUTTER_STAGE(stage),&blue); 

    // Create glTexture to set ClutterActor with rgb565 data

    // Create and add texture actor
    texture = clutter_texture_new();
    GError* error = NULL;
    /*
    texture = clutter_texture_new_from_file("face.png",&error);
    clutter_actor_set_size(texture, GLOBAL_width, GLOBAL_height);
    */
    clutter_container_add_actor(CLUTTER_CONTAINER(stage), texture);
    clutter_actor_show(texture);
    clutter_actor_show(stage);
    //UNUSED(texture);
    if (error != NULL) {
        std::cout << "error" << std::endl;
    }

    // timeline to attach a callback for each frame that is rendered
    timeline = clutter_timeline_new(60); // ms

    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_frame_cb), this);
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb), NULL);

    clutter_timeline_set_loop(timeline, TRUE);
    clutter_timeline_start(timeline);
}

void SharedVideoPlayer::run() {
    g_print("Starting the main loop...\n");
    clutter_main();
}

/// Called from main thread
void SharedVideoPlayer::signalKilled()
{
    boost::mutex::scoped_lock displayLock(displayMutex_);
    killed_ = true;
    textureUploadedCondition_.notify_one(); // in case we're waiting in consumeFrame
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
            //GLOBAL_width = 640;
            //GLOBAL_height = 480;
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
            SharedVideoBuffer *sharedBuffer = static_cast<SharedVideoBuffer*>(addr);
            GLOBAL_width = sharedBuffer->getWidth();
            GLOBAL_height = sharedBuffer->getHeight();
            
            std::cout << "resolution = " << GLOBAL_width << "x" << GLOBAL_height << std::endl;

            clutter_init(&argc, &argv);

            SharedVideoPlayer player;

            // grab the ptr
            player.init(sharedBuffer->pixelsAddress());

            // start our consumer thread, which is a member function of our player object and
            // takes sharedBuffer as an argument
            boost::thread worker(boost::bind<void>(boost::mem_fn(&SharedVideoPlayer::consumeFrame), 
                        boost::ref(player), 
                        sharedBuffer));

            player.run();

            player.signalKilled(); // let worker know that the mainloop has exitted
            worker.join(); // wait for worker to end out before main thread does
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

    return 0;
}

