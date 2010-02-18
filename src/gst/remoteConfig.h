
// remoteConfig.h
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of [propulse]ART.
//
// [propulse]ART is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// [propulse]ART is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with [propulse]ART.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _REMOTE_CONFIG_H_
#define _REMOTE_CONFIG_H_

#include <string>
#include <set>
#include "portOffsets.h"

#include "busMsgHandler.h"

class MapMsg;
class Encoder;
class Pipeline;
class VideoEncoder;
class VideoDecoder;
class Decoder;

/** 
 *      Immutable class that is used to setup rtp
 */

class RemoteConfig 
{
    public:
        RemoteConfig(MapMsg &msg,
                int msgId__); 
        
        virtual ~RemoteConfig(){};
        static bool capsMatchCodec(const std::string &encodingName, const std::string &codec);

        int port() const { return port_; }
        int rtcpFirstPort() const { return port_ + ports::RTCP_FIRST_OFFSET; }
        int rtcpSecondPort() const { return port_ + ports::RTCP_SECOND_OFFSET; }
        int capsPort() const { return port_ + ports::CAPS_OFFSET; }
        const char *remoteHost() const { return remoteHost_.c_str(); }
        bool hasCodec() const { return !codec_.empty(); }
        std::string codec() const { return codec_; }
        void checkPorts() const;
        void cleanupPorts() const;

    protected:

        const std::string codec_;
        const std::string remoteHost_;
        const int port_;
        const int msgId_;
        static const int PORT_MIN;
        static const int PORT_MAX;
        static std::set<int> usedPorts_;

    private:
        RemoteConfig& operator=(const RemoteConfig&); //No Assignment Operator
};

class SenderConfig : public RemoteConfig, public BusMsgHandler
{
    public:
        SenderConfig(Pipeline &pipeline, MapMsg &msg,
                int msgId__);

        VideoEncoder* createVideoEncoder(Pipeline &pipeline, MapMsg &settings) const;
        Encoder* createAudioEncoder(Pipeline &pipeline) const;
        bool capsOutOfBand() { return capsOutOfBand_; }
        void capsOutOfBand(bool capsOutOfBand__) { capsOutOfBand_ = capsOutOfBand__; }

    private:
        static int sendMessage(void *data);

        std::string message_;
        bool capsOutOfBand_;
        bool handleBusMsg(_GstMessage *msg);
};


class ReceiverConfig : public RemoteConfig
{
    public:
        ReceiverConfig(MapMsg &msg, const std::string &caps__,
                int msgId__); 

        VideoDecoder* createVideoDecoder(Pipeline &pipeline, bool doDeinterlace) const;
        Decoder* createAudioDecoder(Pipeline &pipeline) const;

        const char *multicastInterface() const { return multicastInterface_.c_str(); }
        const char *caps() const { return caps_.c_str(); }
        bool capsMatchCodec() const;
        bool hasMulticastInterface() const { return multicastInterface_ != ""; }
        void receiveCaps();
        bool jitterbufferControlEnabled() const { return jitterbufferControlEnabled_; }

    private:
        static bool isSupportedCodec(const std::string &codec);
        const std::string multicastInterface_;
        std::string caps_;
        bool capsOutOfBand_;
        bool jitterbufferControlEnabled_;
};

#endif // _REMOTE_CONFIG_H_

