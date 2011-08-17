/**
 * This example is in the public domain.
 */
#include <boost/bind.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/thread.hpp>
#include <clutter/clutter.h>
#include <iostream>
#include <sharedVideoBuffer.h>
#include <stdlib.h>
#include "clutterexample.h"

#define UNUSED(x) ((void) (x))

class App
{
    public:
        ClutterActor *stage;
};

ClutterActor *SharedVideoPlayer::get_texture()
{
    return texture_;
}

//int framecount = 0;
static const std::string SHARED_MEMORY_ID = "sharedvideoexample";

static void key_event_cb(ClutterActor *actor, ClutterKeyEvent *event, gpointer data)
{
    switch (event->keyval)
    {
        case CLUTTER_Escape:
            clutter_main_quit();
            break;
        default:
            break;
    }
}

void SharedVideoPlayer::on_frame_cb(ClutterTimeline * /*timeline*/, guint * /*ms*/, gpointer data)
{ 
    SharedVideoPlayer* self = (SharedVideoPlayer *) data;

    boost::mutex::scoped_lock displayLock(self->displayMutex_);

    CoglHandle new_texture = COGL_INVALID_HANDLE;
    CoglTextureFlags flags = COGL_TEXTURE_NONE;
    new_texture = cogl_texture_new_from_data(self->get_texture_width(), self->get_texture_height(),
            flags,
            COGL_PIXEL_FORMAT_RGB_565,
            COGL_PIXEL_FORMAT_ANY,
            0,
            self->pixels);
    clutter_texture_set_cogl_texture(CLUTTER_TEXTURE(self->texture_), new_texture);
    cogl_handle_unref(new_texture);

    //framecount++;

    self->textureUploadedCondition_.notify_one();
}

void SharedVideoPlayer::consumeFrame()
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
        boost::mutex::scoped_lock displayLock(displayMutex_);
    }
    // consume all the frames received
    do
    {
        {
            // Lock the mutex
            scoped_lock<interprocess_mutex> lock(sharedBuffer_->getMutex());
            // wait for new buffer to be pushed if it's empty
            if (not sharedBuffer_->waitOnProducer(lock))
            {
                std::cout << "waitOnProducer returned false. Stopping..." << std::endl;
                end_loop = true;
            }
            else
            {
                // got a new buffer, wait until we upload it in gl thread before notifying producer
                {
                    boost::mutex::scoped_lock displayLock(displayMutex_);
                    if (killed_)
                        end_loop = true;
                    else
                        textureUploadedCondition_.wait(displayLock);
                }
                std::cout << "waitOnProducer returned true. got a frame." << std::endl;
                // Notify the other process that the buffer status has changed
                sharedBuffer_->notifyProducer();
            }
            // mutex is released (goes out of scope) here
        }
    }
    while (not end_loop);
    std::cout << "\nWorker thread Going out.\n";
}

SharedVideoPlayer::SharedVideoPlayer() : 
    displayMutex_(),
    textureUploadedCondition_(),
    killed_(false)
{
    texture_ = clutter_texture_new();

    // timeline to attach a callback for each frame that is rendered
    timeline_ = clutter_timeline_new(60); // ms
    g_signal_connect(timeline_, "new-frame", G_CALLBACK(SharedVideoPlayer::on_frame_cb), this);
    clutter_timeline_set_loop(timeline_, TRUE);
    clutter_timeline_start(timeline_);
}

void SharedVideoPlayer::init_pixels(unsigned char *pixelData)
{
    pixels = pixelData;
    // Create glTexture to set ClutterActor with rgb565 data
    // Create and add texture actor
}

/// Called from main thread
void SharedVideoPlayer::signalKilled()
{
    boost::mutex::scoped_lock displayLock(displayMutex_);
    killed_ = true;
    textureUploadedCondition_.notify_one(); // in case we're waiting in consumeFrame
}

bool SharedVideoPlayer::try_open(const std::string &sharedMemoryId)
{
    using namespace boost::interprocess;
    bool opened = false;
    sharedMemoryId_ = sharedMemoryId;

    try
    {
        // open the already created shared memory object
        shared_memory_object shm(open_only, sharedMemoryId_.c_str(), read_write);
        opened = true; 

        // map the whole shared memory in this process
        mapped_region region(shm, read_write);
        // get the address of the region
        void *addr = region.get_address();
        // cast to pointer of type of our shared structure
        sharedBuffer_ = static_cast<SharedVideoBuffer*>(addr);
        texture_width_ = sharedBuffer_->getWidth();
        texture_height_ = sharedBuffer_->getHeight();
        std::cout << "resolution = " << texture_width_ << "x" << texture_height_ << std::endl;
    }
    catch(interprocess_exception &ex)
    {
        static const char *MISSING_ERROR = "No such file or directory";
        if (g_strcmp0(ex.what(), MISSING_ERROR) == 0)
        {
            std::cerr << "Shared buffer " << sharedMemoryId_ << " doesn't exist yet\n";
        }
        else
        {
            shared_memory_object::remove(sharedMemoryId.c_str());
            std::cout << "Unexpected exception: " << ex.what() << std::endl;
            return 1;
        }
    }
    return opened;
}

void SharedVideoPlayer::start_consuming()
{
    // start our consumer thread, which is a member function of our player object and
    // takes sharedBuffer as an argument
    //worker_(
    //    boost::bind<void>(boost::mem_fn(&SharedVideoPlayer::consumeFrame), 
    //    boost::ref(*this), 
    //    sharedBuffer_)
    //    );

    worker_.reset(new boost::thread(
        boost::bind<void>(
            &SharedVideoPlayer::consumeFrame,
            this
        )
    ));
}

void SharedVideoPlayer::tear_down()
{
    signalKilled(); // let worker know that the mainloop has exitted
    worker_.get()->join(); // wait for worker to end out before main thread does
}

int main(int argc, char *argv[])
{
    clutter_init(&argc, &argv);

    bool opened = false;
    std::string sharedMemoryId(SHARED_MEMORY_ID);
    App app;
    SharedVideoPlayer player;

    while (not opened)
    {
        opened = player.try_open(sharedMemoryId);
    }

    /* Get the stage and set its size and color: */
    ClutterColor blue = { 0x00, 0x00, 0xff, 0xff };
    app.stage = clutter_stage_get_default();
    clutter_stage_set_color(CLUTTER_STAGE(app.stage), &blue);
    ClutterActor *texture = player.get_texture();
    clutter_container_add_actor(CLUTTER_CONTAINER(app.stage), texture);
    clutter_actor_show(texture);
    clutter_actor_show(app.stage);
    // grab the ptr
    player.init_pixels(player.getBuffer()->pixelsAddress());
    g_signal_connect(app.stage, "key-press-event", G_CALLBACK(key_event_cb), NULL);
    player.start_consuming();

    g_print("Starting the main loop...\n");
    clutter_main();

    player.tear_down();
    std::cout << "Main thread going out\n";
    return 0;
}

