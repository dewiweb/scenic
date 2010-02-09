/* remoteConfig.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include <algorithm>
#include <boost/assign.hpp>
#include <gst/gst.h>
#include "pipeline.h"
#include "remoteConfig.h"
#include "tcp/asio.h"
#include "mapMsg.h"
#include "codec.h"

const int RemoteConfig::PORT_MIN = 1024;
const int RemoteConfig::PORT_MAX = 65000;


std::set<int> RemoteConfig::usedPorts_;
        
RemoteConfig::RemoteConfig(MapMsg &msg, int msgId__) : 
    codec_(msg["codec"]), remoteHost_(msg["address"]), port_(msg["port"]), msgId_(msgId__)
{}


// Can't be called from destructor, must be called by this object's owner/client, 
// as sometime this object is copied and we don't want the ports destroyed prematurely
void RemoteConfig::cleanupPorts() const
{
    usedPorts_.erase(capsPort());
    usedPorts_.erase(rtcpSecondPort());
    usedPorts_.erase(rtcpFirstPort());
    usedPorts_.erase(port());
}


/// Make sure there are no port clashes (at least as far as this process can tell)
void RemoteConfig::checkPorts() const
{
    if (port_ < PORT_MIN || port_ > PORT_MAX)
        THROW_ERROR("Invalid port " << port_ << ", must be in range [" 
                << PORT_MIN << "," << PORT_MAX << "]");  

    if (usedPorts_.find(port()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", already in use");

    if (usedPorts_.find(rtcpFirstPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its rtcp port " << rtcpFirstPort() << " is already in use");

    if (usedPorts_.find(rtcpSecondPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its rtcp port " << rtcpSecondPort() << " is already in use");

    if (usedPorts_.find(capsPort()) != usedPorts_.end())
        THROW_ERROR("Invalid port " << port() << ", its caps port " << capsPort() << " is already in use");

    // add our ports now that we know they're available
    usedPorts_.insert(port());
    usedPorts_.insert(rtcpFirstPort());
    usedPorts_.insert(rtcpSecondPort());
    usedPorts_.insert(capsPort());
}
        

SenderConfig::SenderConfig(Pipeline &pipeline, MapMsg &msg, int msgId__) : 
    RemoteConfig(msg, msgId__), 
    BusMsgHandler(pipeline),
    message_(""), 
    capsOutOfBand_(false)    // this will be determined later
{}


VideoEncoder * SenderConfig::createVideoEncoder(Pipeline &pipeline, MapMsg &settings) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "h264")
        return new H264Encoder(pipeline, settings);
    else if (codec_ == "h263")
        return new H263Encoder(pipeline, settings);       // set caps from here?
    else if (codec_ == "mpeg4")
        return new Mpeg4Encoder(pipeline, settings);
    else if (codec_ == "theora")
        return new TheoraEncoder(pipeline, settings);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Video encoder " << codec_ << " built"); 
}


Encoder * SenderConfig::createAudioEncoder(Pipeline &pipeline) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make encoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisEncoder(pipeline);
    else if (codec_ == "raw")
        return new RawEncoder(pipeline);
    else if (codec_ == "mp3")
        return new LameEncoder(pipeline);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Audio encoder " << codec_ << " built"); 
}


gboolean SenderConfig::sendMessage(gpointer data) 
{
    const SenderConfig *context = static_cast<const SenderConfig*>(data);
    LOG_DEBUG("\n\n\nSending tcp msg for host " 
            << context->remoteHost_ << " on port " << context->capsPort() 
            << " with id " << context->msgId_);

    /// FIXME: everytime a receiver starts, it should ask sender for caps, then the sender can
    /// send them.
    if (asio::tcpSendBuffer(context->remoteHost_, context->capsPort(), context->msgId_, context->message_))
        LOG_INFO("Caps sent successfully");
    return TRUE;    // try again later, in case we have a new receiver
}


/** 
 * The new caps message is posted on the bus by the src pad of our udpsink, 
 * received by this audiosender, and sent to our other host if needed. */
bool SenderConfig::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    if (s != NULL and gst_structure_has_name(s, "caps-changed"))
    {   
        // this is our msg
        const gchar *newCapsStr = gst_structure_get_string(s, "caps");
        tassert(newCapsStr);
        std::string str(newCapsStr);

        GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(str.c_str()), 0);
        const GValue *encodingStr = gst_structure_get_value(structure, "encoding-name");
        std::string encodingName(g_value_get_string(encodingStr));

        if (!capsMatchCodec(encodingName, codec()))
            return false;   // not our caps, ignore it
        else if (capsOutOfBand_) 
        { 
            LOG_DEBUG("Sending caps for codec " << codec());

            message_ = std::string(newCapsStr);
            enum {MESSAGE_SEND_TIMEOUT = 5000}; // ms
            g_timeout_add(MESSAGE_SEND_TIMEOUT, static_cast<GSourceFunc>(SenderConfig::sendMessage), 
                    static_cast<gpointer>(this));
            return true;
        }
        else
            return true;       // was our caps, but we don't need to send caps for it
    }

    return false;           // this wasn't our msg, someone else should handle it
}


bool ReceiverConfig::isSupportedCodec(const std::string &codec)
{
    using namespace boost::assign;
    using std::string;
    
    static const std::vector<string> CODECS = 
        list_of<string>("mpeg4")("theora")("vorbis")("raw")("h264")("h263")("mp3");

    bool result = std::find(CODECS.begin(), CODECS.end(), codec) != CODECS.end();
    return result;
}


ReceiverConfig::ReceiverConfig(MapMsg &msg,
        const std::string &caps__,
        int msgId__) : 
    RemoteConfig(msg, msgId__), 
    multicastInterface_(msg["multicast-interface"]), caps_(caps__), 
    capsOutOfBand_(msg["negotiate-caps"] or caps_ == ""),
    jitterbufferControlEnabled_(msg["enable-controls"])
{
    if (capsOutOfBand_) // couldn't find caps, need them from other host or we've explicitly been told to send caps
    {
        if (isSupportedCodec(codec_))   // this would fail later but we want to make sure we don't wait with a bogus codec
        { 
            LOG_INFO("Waiting for " << codec_ << " caps from other host");
            receiveCaps();  // wait for new caps from sender
        }
        else
            THROW_ERROR("Codec " << codec_ << " is not supported");
    }
}

VideoDecoder * ReceiverConfig::createVideoDecoder(Pipeline &pipeline, bool doDeinterlace) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "h264")
        return new H264Decoder(pipeline, doDeinterlace);
    else if (codec_ == "h263")
        return new H263Decoder(pipeline, doDeinterlace);
    else if (codec_ == "mpeg4")
        return new Mpeg4Decoder(pipeline, doDeinterlace);
    else if (codec_ == "theora")
        return new TheoraDecoder(pipeline, doDeinterlace);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Video decoder " << codec_ << " built"); 
}


Decoder * ReceiverConfig::createAudioDecoder(Pipeline &pipeline) const
{
    if (codec_.empty())
        THROW_ERROR("Can't make decoder without codec being specified.");

    if (codec_ == "vorbis")
        return new VorbisDecoder(pipeline);
    else if (codec_ == "raw")
        return new RawDecoder(pipeline);
    else if (codec_ == "mp3")
        return new MadDecoder(pipeline);
    else
    {
        THROW_ERROR(codec_ << " is an invalid codec!");
        return 0;
    }
    LOG_DEBUG("Audio decoder " << codec_ << " built"); 
}


/// compares internal codec names and RTP-header codec names
bool RemoteConfig::capsMatchCodec(const std::string &encodingName, const std::string &codec)
{
    return (encodingName == "VORBIS" and codec == "vorbis")
    or (encodingName == "L16" and codec == "raw")
    or (encodingName == "MPA" and codec == "mp3")
    or (encodingName == "MP4V-ES" and codec == "mpeg4")
    or (encodingName == "H264" and codec == "h264")
    or (encodingName == "H263-1998" and codec == "h263")
    or (encodingName == "THEORA" and codec == "theora");
}

/// This function makes sure that the caps set on this receiver by a sender, match the codec
/// that it expects. If it fails, it is probably due to a mismatch of codecs between sender and
/// receiver, or a change in the caps specification for a given codec from gstreamer.
//  TODO: maybe the whole receiver should be created based on this info, at least up to and including
//  the decoder?
bool ReceiverConfig::capsMatchCodec() const
{
    GstStructure *structure = gst_caps_get_structure(gst_caps_from_string(caps_.c_str()), 0);
    const GValue *str = gst_structure_get_value(structure, "encoding-name");
    std::string encodingName(g_value_get_string(str));

    return RemoteConfig::capsMatchCodec(encodingName, codec_);
}


void ReceiverConfig::receiveCaps()
{
    int id;
    // this blocks
    std::string msg(asio::tcpGetBuffer(capsPort(), id));
    //tassert(id == msgId_);
    caps_ = msg;
}

