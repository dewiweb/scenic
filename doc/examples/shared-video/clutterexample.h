#include <boost/thread/thread.hpp>
#include <boost/thread/condition.hpp>
#include <clutter/clutter.h>
#include <tr1/memory>

class SharedVideoBuffer;

class SharedVideoPlayer
{
    private:
        ClutterActor *texture_;
        int texture_width_;
        int texture_height_;
        SharedVideoBuffer *sharedBuffer_;
        std::tr1::shared_ptr<boost::thread> worker_;
        std::string sharedMemoryId_;
    public:
        ClutterTimeline *timeline_;
        boost::mutex displayMutex_;
        boost::condition_variable textureUploadedCondition_;
        bool killed_;
        unsigned char *pixels;

        SharedVideoPlayer();
        void signalKilled();
        void init_pixels(unsigned char *pixelData);
        void consumeFrame();
        ClutterActor *get_texture();
        static void on_frame_cb(ClutterTimeline *timeline, guint *ms, gpointer data);
        int get_texture_width() { return texture_width_; }
        int get_texture_height() { return texture_height_; }
        void tear_down();
        void start_consuming();
        bool try_open(const std::string &sharedMemoryId);
        SharedVideoBuffer *getBuffer() { return sharedBuffer_; }
};

