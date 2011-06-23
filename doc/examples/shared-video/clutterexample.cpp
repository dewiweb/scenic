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

/// FUCKING GROSS!!!
static int GLOBAL_width = 0;
static int GLOBAL_height = 0;
int framecount = 0;

static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event, gpointer data) {
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

static void on_frame_cb(ClutterTimeline *timeline, guint *ms, gpointer data) { 
    boost::mutex::scoped_lock displayLock(((SharedVideoPlayer*)data)->displayMutex_);

    CoglHandle new_texture = COGL_INVALID_HANDLE;
    CoglTextureFlags flags = COGL_TEXTURE_NONE;
    new_texture = cogl_texture_new_from_data(GLOBAL_width,GLOBAL_height,
            flags,
            COGL_PIXEL_FORMAT_RGB_565,
            COGL_PIXEL_FORMAT_ANY,
            0,
            ((SharedVideoPlayer*)data)->pixels);
    clutter_texture_set_cogl_texture(CLUTTER_TEXTURE(((SharedVideoPlayer*)data)->texture), new_texture);
    cogl_handle_unref(new_texture);

    UNUSED(timeline);
    UNUSED(ms);

    framecount++;

    ((SharedVideoPlayer*)data)->textureUploadedCondition_.notify_one();
}

void SharedVideoPlayer::consumeFrame(SharedVideoBuffer *sharedBuffer) {
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::shared_memory_object;

    std::cout << "\nWorker thread started.\n";

    // get frames until the other process marks the end
    bool end_loop = false;

    // make sure there's no sentinel
    {
        // Lock the mutex
        boost::mutex::scoped_lock displayLock(displayMutex_);
    }


    do {
        {
            // Lock the mutex
            scoped_lock<interprocess_mutex> lock(sharedBuffer->getMutex());

            // wait for new buffer to be pushed if it's empty
            if (not sharedBuffer->waitOnProducer(lock)) {
                end_loop = true;
            } else {
                // got a new buffer, wait until we upload it in gl thread before notifying producer
                {
                    boost::mutex::scoped_lock displayLock(displayMutex_);

                    if (killed_) {
                        end_loop = true;
                    } else {
                        textureUploadedCondition_.wait(displayLock);
                    }
                }

                // Notify the other process that the buffer status has changed
                sharedBuffer->notifyProducer();
            }
            // mutex is released (goes out of scope) here
        }
    } while (!end_loop);

    std::cout << "\nWorker thread Going out.\n";
}

SharedVideoPlayer::SharedVideoPlayer() : 
    displayMutex_(), textureUploadedCondition_(), killed_(false)
{}

void SharedVideoPlayer::init(unsigned char *pixelData) {

    pixels = pixelData;
    ClutterColor blue = { 0x00, 0x00, 0xff, 0xff };

    /* Get the stage and set its size and color: */
    stage = clutter_stage_get_default();
    clutter_stage_set_color(CLUTTER_STAGE(stage),&blue); 

    // Create glTexture to set ClutterActor with rgb565 data

    // Create and add texture actor
    texture = clutter_texture_new();

    clutter_container_add_actor(CLUTTER_CONTAINER(stage), texture);
    clutter_actor_show(texture);
    clutter_actor_show(stage);

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
void SharedVideoPlayer::signalKilled() {
    boost::mutex::scoped_lock displayLock(displayMutex_);
    killed_ = true;
    textureUploadedCondition_.notify_one(); // in case we're waiting in consumeFrame
}

int main(int argc, char *argv[]) {
    using namespace boost::interprocess;

    std::string sharedMemoryId("shared_memory");
    switch (argc) {
        case 2: // just id
            sharedMemoryId = std::string(argv[1]);
            break;
    }

    bool opened = false;
    while (!opened) {
        try {
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
        } catch(interprocess_exception &ex) {
            static const char *MISSING_ERROR = "No such file or directory";

            if (strncmp(ex.what(), MISSING_ERROR, strlen(MISSING_ERROR)) != 0) {
                shared_memory_object::remove(sharedMemoryId.c_str());
                std::cout << "Unexpected exception: " << ex.what() << std::endl;
                return 1;
            } else {
                std::cerr << "Shared buffer doesn't exist yet\n";
            }
        }
    }   // end while not opened

    return 0;
}
