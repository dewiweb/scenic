
// audioBase.h
#ifndef _AUDIO_BASE_H_
#define _AUDIO_BASE_H_

typedef struct _GstElement GstElement;

class AudioBase 
{
    public:
        virtual bool start();
        bool stop();
        bool isPlaying();
        int port() const { return port_; }

    protected:
        AudioBase();
        virtual ~AudioBase();
        int port_;
        static const int DEF_PORT;
        GstElement *pipeline_;

    private:
};

#endif // _AUDIO_BASE_H_
