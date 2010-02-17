
// videoSender.h
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

#ifndef _VIDEO_SENDER_H_
#define _VIDEO_SENDER_H_

#include "mediaBase.h"
#include "videoConfig.h"
#include "rtpSender.h"
#include "busMsgHandler.h"

#include "noncopyable.h"

#include <boost/shared_ptr.hpp>

class VideoSource;
class VideoEncoder;
class Pay;
class _GstMessage;

class VideoSender
    : public SenderBase, boost::noncopyable
{
    public:
        VideoSender(Pipeline &pipeline,
                boost::shared_ptr<VideoSourceConfig> vConfig, 
                boost::shared_ptr<SenderConfig> rConfig);
        ~VideoSender();

    private:
        void createSource(Pipeline &pipeline);
        void createCodec(Pipeline &pipeline);
        void createPayloader();
        virtual bool checkCaps() const;

        boost::shared_ptr<VideoSourceConfig> videoConfig_;
        RtpSender session_;
        VideoSource *source_;
        VideoEncoder *encoder_;
        Pay *payloader_; 
};

#endif
