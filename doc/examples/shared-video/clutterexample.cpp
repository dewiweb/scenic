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

//static int __aRBLookUpTable[32] = {5,18,29,38,48,56,64,71,78,85,92,99,106,113,119,125,131,137,144,151,157,164,171,177,184,192,200,208,216,226,238,250};
//static int __aGLookUpTable[64] = {3,10,16,22,27,32,36,41,46,50,54,57,61,65,69,73,77,80,83,86,90,93,96,100,103,106,109,113,116,119,123,126,130,133,136,139,142,145,149,152,155,158,162,165,168,172,175,178,182,185,189,193,196,200,205,209,213,218,222,227,233,239,246,252};

/// FUCKING GROSS!!!
static int GLOBAL_width = 0;
static int GLOBAL_height = 0;
static SharedVideoBuffer *sharedBuffer;
CoglHandle glTexture;
unsigned char rgbPixels[640*480*3];
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
    /*unsigned short* pSrcBitmap;
    pSrcBitmap = (unsigned short*)sharedBuffer->pixelsAddress();
    unsigned short srcPixel;
    //unsigned char* pTargetBitmap;
    //pTargetBitmap = &rgbPixels[0];
    unsigned char r=0,g=0,b=0;
    */

    //copy
    /*
    for(int y=0; y < GLOBAL_height; y++)
    {
        for(int x=0; x < GLOBAL_width; x++)
        {
            srcPixel = (unsigned short)*pSrcBitmap++;
            // get indexes
            r = (srcPixel >> 11) & 31;
            g = (srcPixel >> 5 ) & 63;
            b = (srcPixel & 31);


            // store new values as rgb888
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+0] = (unsigned char)255; 
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+1] = (unsigned char)0; 
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+2] = (unsigned char)0; 
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+0] = __aRBLookUpTable[r];
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+1] = __aGLookUpTable[g];
            rgbPixels[(y*GLOBAL_width*3)+(x*3)+2] = __aRBLookUpTable[b];
        }
    }
    */


    /*
    short red_mask = 0xF800;
    short green_mask = 0x7E0;
    short blue_mask = 0x1F;
    unsigned char red_value;
    unsigned char green_value;
    unsigned char blue_value;
    */
    // rgb565 -> rgb888
    //x8 = 255/31 * x5
    //x8 = 255/63 * x6
    /*
    short fiveSixFive;
    for (int i=0;i<640*480;i++) {
        fiveSixFive = (sharedBuffer->pixelsAddress()[i] >> 8) & (sharedBuffer->pixelsAddress()[i] << 8);
        red_value = (fiveSixFive & red_mask) >> 11;
        green_value = (fiveSixFive & green_mask) >> 5;
        blue_value = (fiveSixFive & blue_mask);
        rgbPixels[(i*3)+0] = red_value;
        rgbPixels[(i*3)+1] = red_value;
        rgbPixels[(i*3)+2] = red_value;
    }
    */

    bool feedback = clutter_texture_set_from_rgb_data(CLUTTER_TEXTURE(data),sharedBuffer->pixelsAddress(),false,GLOBAL_width,GLOBAL_height,0,3,CLUTTER_TEXTURE_NONE,NULL);
    if (!feedback) {
        std::cout << "failed" << std::endl;
    }
    clutter_actor_set_size(CLUTTER_ACTOR(data), GLOBAL_width, GLOBAL_height);
    /*
    GError* error = NULL;
    data = clutter_texture_new_from_file("face.png",&error);
    //UNUSED(texture);
    if (error != NULL) {
        std::cout << "error" << std::endl;
    }
    */

/*
    glTexture = cogl_texture_new_from_data(
            GLOBAL_width,
            GLOBAL_height,
            COGL_TEXTURE_NONE,
            COGL_PIXEL_FORMAT_RGB_888,
            COGL_PIXEL_FORMAT_RGB_888,
            GLOBAL_width*2,
            sharedBuffer->pixelsAddress());            
    */
    /*
    glTexture = cogl_texture_new_from_data(
            GLOBAL_width,
            GLOBAL_height,
            COGL_TEXTURE_NONE,
            COGL_PIXEL_FORMAT_RGB_888,
            COGL_PIXEL_FORMAT_RGB_888,
            GLOBAL_width*3,
            pTargetBitmap);            
    clutter_texture_set_cogl_texture(CLUTTER_TEXTURE(data),glTexture);
    */
    UNUSED(timeline);
    UNUSED(ms);


    //g_print("on_frame_cb\n");
    framecount++;
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
            sharedBuffer = static_cast<SharedVideoBuffer*>(addr);
            GLOBAL_width = sharedBuffer->getWidth();
            GLOBAL_height = sharedBuffer->getHeight();
            
            std::cout << "resolution = " << GLOBAL_width << "x" << GLOBAL_height << std::endl;

            std::cout << "sizeof(short)" << sizeof(short) << std::endl;
            std::cout << "sizeof(unsigned char)" << sizeof(unsigned char) << std::endl;
            /*
            for (int i=6000;i<6100;i++) {
                std::cout << "buffer[" << i << "] " << int(sharedBuffer->pixelsAddress()[i]) << std::endl;
            }
            */

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
    ClutterActor *stage = NULL;
    ClutterActor *texture = NULL;
    ClutterTimeline *timeline = NULL;

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
    //UNUSED(texture);
    if (error != NULL) {
        std::cout << "error" << std::endl;
    }

    // timeline to attach a callback for each frame that is rendered
    timeline = clutter_timeline_new(100000); // ms
    clutter_timeline_set_loop(timeline, TRUE);
    clutter_timeline_start(timeline);
    

    g_signal_connect(timeline, "new-frame", G_CALLBACK(on_frame_cb), texture);
    g_signal_connect(stage, "key-press-event", G_CALLBACK(key_event_cb), NULL);

    g_print("Starting the main loop...\n");
    /* Start the main loop, so we can respond to events: */
    clutter_actor_show_all(stage);
    clutter_main();

    return EXIT_SUCCESS;
}

