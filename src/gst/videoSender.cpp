/* videoSender.cpp
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

#include "pipeline.h"
#include "gstLinkable.h"
#include "videoSender.h"
#include "videoSource.h"
#include "rtpPay.h"
#include "videoConfig.h"
#include "remoteConfig.h"
#include "codec.h"
#include "caps/capsParser.h"
#include "messageDispatcher.h"

#include <tr1/memory>

using std::tr1::shared_ptr;

/// Constructor
VideoSender::VideoSender(Pipeline &pipeline,
        const shared_ptr<VideoSourceConfig> &vConfig,
        const shared_ptr<SenderConfig> &rConfig) :
    SenderBase(rConfig),
    videoConfig_(vConfig),
    session_(pipeline),
    source_(),
    encoder_(),
    payloader_()
{
    createPipeline(pipeline);
}

bool VideoSender::checkCaps() const
{
    return CapsParser::getVideoCaps(remoteConfig_->codec(), 
            videoConfig_->captureWidth(), videoConfig_->captureHeight(),
            videoConfig_->pictureAspectRatio()) != ""; 
}

VideoSender::~VideoSender()
{}


void VideoSender::createSource(Pipeline &pipeline)
{
    source_.reset(videoConfig_->createSource(pipeline));
    assert(source_);
}


void VideoSender::createCodec(Pipeline &pipeline)
{
    encoder_.reset(remoteConfig_->createVideoEncoder(pipeline, videoConfig_->bitrate(), videoConfig_->quality()));
    assert(encoder_);
    bool linked = false;
    int framerateIndex = 0;
    while (not linked)
    {
      try 
	{
	  tee_ = pipeline.makeElement("tee", NULL);
	  // localpreview_ = pipeline.makeElement("xvimagesink", NULL);
	  // g_object_set (G_OBJECT (localpreview_), "sync", FALSE, NULL);
	  lpqueue_ = pipeline.makeElement("queue", NULL);
	  std::stringstream strm;
	  std::string port;
	  strm << remoteConfig_->port();
	  strm >> port;
	  std::string localShmName("dieudj");
	  localShmName.append(port);
	  localshvid_.reset(new SharedVideoSink(pipeline, source_->getWidth() , source_->getHeight(),localShmName));
	  gstlinkable::link(*source_,tee_); 
	  gstlinkable::link(tee_,  *encoder_);
	  gstlinkable::link(tee_, lpqueue_);  
	  //gstlinkable::link(lpqueue_,localpreview_);
	  gstlinkable::link(lpqueue_,*localshvid_);
 
	  linked = true;
        }
        catch (const gstlinkable::LinkExcept &e)
        {
            LOG_WARNING("Link failed, trying another framerate");
            ++framerateIndex;
            source_->setCapsFilter(source_->srcCaps(framerateIndex));
        }
    }
}


void VideoSender::createPayloader()       
{
    payloader_.reset(encoder_->createPayloader());
    assert(payloader_);
    // tell rtpmp4vpay not to send config string in header since we're sending caps
    if (remoteConfig_->capsOutOfBand() and remoteConfig_->codec() == "mpeg4") 
        MessageDispatcher::sendMessage("disable-send-config");
    gstlinkable::link(*encoder_, *payloader_);
    session_.add(payloader_.get(), *remoteConfig_);
}

