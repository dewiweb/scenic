/******************************************************************************\
 * OSG viewer of our shared video buffer                                       *
 * Copyright Tristan Matthews
 * tristan@sat.qc.ca
 \******************************************************************************/


#include <osg/Drawable>
#include <osg/Geometry>
#include <osg/TexMat>
#include <osg/TexEnv>
#include <osg/TextureRectangle>
#include <osgViewer/View>
#include <osgViewer/ViewerEventHandlers>
#include <osg/GraphicsContext>

#include <boost/bind.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>

#include "./osgview.h"
#include "sharedVideoBuffer.h"

static const GLenum PIXEL_TYPE = GL_UNSIGNED_SHORT_5_6_5;

/// GROSS!!!
static int GLOBAL_width = 0;
static int GLOBAL_height = 0;


TextureUpdateCallback::TextureUpdateCallback(SharedVideoPlayer &player, 
        unsigned char *pixelData, 
        osg::ref_ptr<osg::Image> videoImage, 
        osg::ref_ptr<osg::TextureRectangle> videoTexture) :
    player_(player),
    pixelData_(pixelData), 
    videoImage_(videoImage),
    videoTexture_(videoTexture)  
{}


/// Guaranteed to be called while not drawing the texture, called from main thread
void TextureUpdateCallback::operator() (osg::Node*, osg::NodeVisitor *nv)
{
    boost::mutex::scoped_lock displayLock(player_.displayMutex_);

    // do update here

    // update image from shared memory:
    videoImage_->setImage(GLOBAL_width,
            GLOBAL_height,
            0, 
            GL_RGB,
            GL_RGB,
            PIXEL_TYPE, 
            pixelData_, 
            osg::Image::NO_DELETE, 
            1);

    // set texture:
    videoTexture_->setImage(videoImage_.get());
    player_.textureUploadedCondition_.notify_one();
}


SharedVideoPlayer::SharedVideoPlayer() : 
    viewer_(), displayMutex_(), textureUploadedCondition_(), killed_(false)
{}


/// Called from main thread
void SharedVideoPlayer::signalKilled()
{
    boost::mutex::scoped_lock displayLock(displayMutex_);
    killed_ = true;
    textureUploadedCondition_.notify_one(); // in case we're waiting in consumeFrame
}


/// This function is executed in the worker thread
void SharedVideoPlayer::consumeFrame(SharedVideoBuffer *sharedBuffer)
{
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::shared_memory_object;

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

            //if (!sharedBuffer->isPushing())
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
                        //sharedBuffer->stopPushing();   // tell appsink not to give us any more buffers
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


osg::ref_ptr<osg::Geode> createRectangle(SharedVideoPlayer &player, unsigned char *pixelData)
{ 
    using namespace osg;

    // create geometry(centre vector, widthVector, heightVector)
    ref_ptr<Geometry> geometry(createTexturedQuadGeometry(Vec3(0.0f, 0.0f, 0.0), 
                Vec3(GLOBAL_width, 0.0f, 0.0), 
                Vec3(0.0f, 0.0f, -GLOBAL_height))); // flip because video y-axis is reversed from gl's

    // disable display list so our modified tex coordinates show up
    //geometry->setUseDisplayList(false);

    // create image
    ref_ptr<Image> img(new Image);

    // setup texture
    ref_ptr<TextureRectangle> texture(new TextureRectangle(img.get()));
    texture->setFilter(Texture::MIN_FILTER, Texture::NEAREST);
    texture->setFilter(Texture::MAG_FILTER, Texture::NEAREST);
    texture->setWrap(Texture::WRAP_S, Texture::CLAMP_TO_EDGE);
    texture->setWrap(Texture::WRAP_T, Texture::CLAMP_TO_EDGE);

    ref_ptr<TexMat> texmat(new TexMat);
    texmat->setScaleByTextureRectangleSize(true);

    ref_ptr<TexEnv> texenv(new TexEnv);
    texenv->setMode(TexEnv::REPLACE);

    // setup state for geometry
    ref_ptr<StateSet> state(geometry->getOrCreateStateSet());
    state->setTextureAttributeAndModes(0, texture.get(), StateAttribute::ON);
    state->setTextureAttributeAndModes(0, texmat.get(), StateAttribute::ON);
    state->setTextureAttributeAndModes(0, texenv.get(), StateAttribute::ON);

    // turn off lighting 
    state->setMode(GL_LIGHTING, StateAttribute::OFF);

    // install 'update' callback
    ref_ptr<Geode> geode(new Geode);
    geode->addDrawable(geometry.get());
    geode->setUpdateCallback(new TextureUpdateCallback(player, pixelData, img, texture));

    return geode;
}


osg::ref_ptr<osg::Group> createModel(SharedVideoPlayer& player, unsigned char *pixelData)
{
    using namespace osg;

    ref_ptr<Group> root(new Group);
    root->addChild(createRectangle(player, pixelData).get()); 
    return root;
}


void SharedVideoPlayer::init(unsigned char *pixelData)
{
    using namespace osg;
    // Create the scene graph. 
    // Declare a group to act as root node of a scene:
    ref_ptr<Group> sgRoot(createModel(*this, pixelData));


	// *************************************************************************
    // window and graphicsContext stuff:
    GraphicsContext::WindowingSystemInterface* wsi = GraphicsContext::getWindowingSystemInterface();

    if (!wsi)
    {
        notify(NOTICE)<<"Error, no WindowSystemInterface available," 
            "cannot create windows."<<std::endl;
        exit(0);
    }


    unsigned int width, height;
    wsi->getScreenResolution(GraphicsContext::ScreenIdentifier(0), width, height);

    // create a GraphicsContext::Traits for this window 
    // and initialize with some defaults:
	ref_ptr<GraphicsContext::Traits> gfxTraits = new GraphicsContext::Traits;

    gfxTraits->windowName = "Milhouse Viewer";
	gfxTraits->x = 50;
	gfxTraits->y = 50;
	gfxTraits->width = GLOBAL_width;
	gfxTraits->height = GLOBAL_height;
	gfxTraits->windowDecoration = true;
	gfxTraits->doubleBuffer = true;
	gfxTraits->useCursor = true;
	gfxTraits->supportsResize = true;
	gfxTraits->sharedContext = 0;

	gfxTraits->displayNum = 0;
	gfxTraits->screenNum = 0;

	ref_ptr<GraphicsContext> gfxContext = 
        GraphicsContext::createGraphicsContext(gfxTraits.get());

    if (gfxContext.valid())
    {
        gfxContext->setClearColor(Vec4f(0.0f,0.0f,0.0f,1.0f));
        gfxContext->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } 
    else 
        std::cout << "ERROR: Could not create GraphicsContext." << std::endl;

    viewer_.getCamera()->setClearColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f)); // black
    viewer_.getCamera()->setGraphicsContext(gfxContext.get());
    viewer_.getCamera()->setViewport(new Viewport(0,0, 
                gfxTraits->width, gfxTraits->height));

    // Set the data to the viewer
    viewer_.setSceneData(sgRoot.get());
}


void SharedVideoPlayer::run()
{
    viewer_.run();
}


// shared memory must exist throughout process
int main (int argc, char* argv[])
{
    using namespace boost::interprocess;

    std::string sharedMemoryId("shared_memory");
    switch (argc)
    {
        case 2: // just id
            sharedMemoryId = std::string(argv[1]);
            break;
        case 3: // just resolution
            GLOBAL_width = boost::lexical_cast<int>(argv[1]);
            GLOBAL_height = boost::lexical_cast<int>(argv[2]);
            break;
        case 4: // id and resolution
            sharedMemoryId = std::string(argv[1]);
            GLOBAL_width = boost::lexical_cast<int>(argv[2]);
            GLOBAL_height = boost::lexical_cast<int>(argv[3]);
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
            SharedVideoBuffer *sharedBuffer = static_cast<SharedVideoBuffer*>(addr);
            GLOBAL_width = sharedBuffer->getWidth();
            GLOBAL_height = sharedBuffer->getHeight();
            
            std::cout << "resolution = " << GLOBAL_width << "x" << GLOBAL_height << std::endl;

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
                boost::this_thread::sleep(boost::posix_time::milliseconds(30)); 
            }
        }
    }   // end while not opened

    return 0;
}

