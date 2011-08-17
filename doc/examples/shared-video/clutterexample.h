#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <clutter/clutter.h>

class SharedVideoBuffer;

class SharedVideoPlayer
{
    private:
        ClutterActor *texture_;
    public:
        ClutterTimeline *timeline;
        boost::mutex displayMutex_;
        boost::condition_variable textureUploadedCondition_;
        bool killed_;
        unsigned char *pixels;

        SharedVideoPlayer();
        void signalKilled();
        void init(unsigned char *pixelData);
        void consumeFrame(SharedVideoBuffer *sharedBuffer);
        ClutterActor *get_texture();
        static void on_frame_cb(ClutterTimeline *timeline, guint *ms, gpointer data);
};

