/* sropulpof.cpp
 * Copyright 2008 Koya Charles & Tristan Matthews 
 *
 * This file is part of [propulse]ART.
 *
 * [propulse]ART is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * [propulse]ART is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "util.h"

#include "gutil.h"
#include "msgThreadFactory.h"

#define USE_SMART_PTR //Factories return a shared_ptr 
#include "gst/videoFactory.h"
#include "gst/audioFactory.h"

#define BLOCK() gutil::runMainLoop(0);

namespace pof 
{
    short run(int argc, char **argv);
    const short NUM_CHANNELS = 2;

#ifdef CONFIG_BOOST
    using namespace boost;
#else
    using namespace std::tr1;
#endif
}

int telnetServer(int,int);

// 2way audio and video
short pof::run(int argc, char **argv)
{
    char pid;
    bool send = false;
    bool recv = false;
    bool full = false;
    bool disableAudio = false;
    bool disableVideo = false;
    OptionArgs options;
    char *ip = 0;
    char *videoCodec = 0;
    char *audioCodec = 0;
    char *videoSink = 0;
    int audioPort = 0;
    int videoPort = 0;
    int videoBitrate = 3000000;
    char *videoDevice = 0;

    int screenNum = 0;
    int numChannels = NUM_CHANNELS;
    
    bool version = false;

    options.add(new BoolArg(&send,"sender", 's', "sender"));
    options.add(new BoolArg(&recv,"receiver", 'r', "receiver"));
    options.add(new StringArg(&ip, "address", 'i', "address", "provide ip address"));
    options.add(new StringArg(&videoCodec, "videocodec", 'v', "videocodec", "h264"));
    options.add(new StringArg(&audioCodec, "audiocodec", 'a', "audiocodec", "vorbis raw mp3"));
    options.add(new StringArg(&videoSink, "videosink", 'k', "videosink", "xvimagesink glimagesink"));
    options.add(new IntArg(&audioPort, "audioport", 't', "audioport", "portnum"));
    options.add(new IntArg(&videoPort, "videoport", 'p', "videoport", "portnum"));
    options.add(new BoolArg(&full,"fullscreen", 'f', "default to fullscreen"));
    options.add(new BoolArg(&disableAudio,"disableaudio", 'y', "disable audio"));
    options.add(new BoolArg(&disableVideo,"disablevideo", 'z', "disable video"));
    options.add(new StringArg(&videoDevice, "videodevice", 'd', "device", "/dev/video0 /dev/video1"));
    options.add(new IntArg(&screenNum, "screen", 'n', "screen", "xinerama screen num"));
    options.add(new BoolArg(&version, "version", '\0', "version number"));
    options.add(new IntArg(&numChannels, "numChannels", 'c', "numChannels", "2"));
    options.add(new IntArg(&videoBitrate, "videobitrate", 'x', "videobitrate", "3000000"));

    //telnetServer param
    int serverport=0;
    options.add(new IntArg(&serverport, "serverport", '\0', "run as server", "port to listen on"));

    options.parse(argc, argv);

    if(version)
    {
        LOG_INFO("version " << PACKAGE_VERSION << '\b' << RELEASE_CANDIDATE);
        return 0;
    }

    if (send)
        pid = 's';
    else if (recv)
        pid = 'r';
    else
        THROW_ERROR("argument error: must be sender or receiver. see --help");

    LOG_INFO("Built on " << __DATE__ << " at " << __TIME__);

    if(serverport)
        return telnetServer(pid == 's', serverport);

    if(ip == 0) 
        THROW_ERROR("argument error: missing ip. see --help");
    if(!disableVideo && videoCodec == 0)
        THROW_ERROR("argument error: missing videoCodec. see --help");
    if (disableVideo && disableAudio)
        THROW_ERROR("argument error: can't disable video and audio. see --help");

    if (pid == 'r') {
        shared_ptr<VideoReceiver> vRx;
        shared_ptr<AudioReceiver> aRx;

        if (!disableVideo)
        {
            if(videoSink == 0)
                THROW_ERROR("argument error: missing videoSink. see --help");

            vRx = videofactory::buildVideoReceiver(ip, videoCodec, videoPort, screenNum, videoSink);
        }
        if (!disableAudio)
            aRx = audiofactory::buildAudioReceiver(ip, audioCodec, audioPort);

#ifdef CONFIG_DEBUG_LOCAL
        playback::makeVerbose();
#endif

        playback::start();

        if (!disableVideo)
        {
            if(full)
                vRx->makeFullscreen();
        }

        BLOCK();
        assert(playback::isPlaying());

        playback::stop();
    }
    else {
        shared_ptr<VideoSender> vTx;
        shared_ptr<AudioSender> aTx;

        if (!disableVideo)
        {
            VideoSourceConfig *vConfig; 

            if (videoDevice)
                vConfig = new VideoSourceConfig("v4l2src", videoBitrate, videoDevice);
            else
                vConfig = new VideoSourceConfig("v4l2src", videoBitrate);

            vTx = videofactory::buildVideoSender(*vConfig, ip, videoCodec, videoPort);
            delete vConfig;
        }

        if (!disableAudio)
        {
            AudioSourceConfig aConfig("jackaudiosrc", numChannels);
            aTx = audiofactory::buildAudioSender(aConfig, ip, audioCodec, audioPort);
        }

#ifdef CONFIG_DEBUG_LOCAL
        playback::makeVerbose();
#endif

        playback::start();

        if (!disableVideo)
            assert(tcpSendBuffer(ip, ports::VIDEO_CAPS_PORT, videofactory::MSG_ID, vTx->getCaps()));
        if (!disableAudio)
            assert(tcpSendBuffer(ip, ports::AUDIO_CAPS_PORT, audiofactory::MSG_ID, aTx->getCaps()));

        BLOCK();
        assert(playback::isPlaying());

        playback::stop();
    }
    return 0;
}


int main(int argc, char **argv)
{
    try {
        return pof::run(argc, argv);
    }
    catch (Except e)
    {
        //std::cerr << e.msg_;
        return 1;
    }
    return 0;
}

