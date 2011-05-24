#ifndef _OSG_VIEWER_H_
#define _OSG_VIEWER_H_

#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>


class SharedVideoBuffer;

class SharedVideoPlayer
{
    friend class TextureUpdateCallback;
    private:
        osgViewer::Viewer viewer_;
        boost::mutex displayMutex_;
        boost::condition_variable textureUploadedCondition_;
        bool killed_;
    public:
        SharedVideoPlayer();
        void signalKilled();
        void init(unsigned char *pixelData);
        void run();
        void consumeFrame(SharedVideoBuffer *sharedBuffer);
};


class TextureUpdateCallback : public osg::NodeCallback
{
    public:
        TextureUpdateCallback(SharedVideoPlayer &player, 
                unsigned char *pixelData, 
                osg::ref_ptr<osg::Image> videoImage, 
                osg::ref_ptr<osg::TextureRectangle> videoTexture);

        virtual void operator()(osg::Node*, osg::NodeVisitor* nv);

    private:
        SharedVideoPlayer &player_;
        unsigned char * pixelData_;
        osg::ref_ptr<osg::Image> videoImage_;
        osg::ref_ptr<osg::TextureRectangle> videoTexture_;
};




#endif // _OSG_VIEWER_H_
