/* audioReceiver.h
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
#ifndef _AUDIO_RECEIVER_H_
#define _AUDIO_RECEIVER_H_

#include "mediaBase.h"
#include "audioConfig.h"
#include "remoteConfig.h"
#include "rtpReceiver.h"
#include "audioLevel.h"
#include "messageHandler.h"

#include "noncopyable.h"

class RtpPay;
class Decoder;
class AudioSink;

/** 
 * An audio pipeline derived from ReceiverBase that receives audio over rtp,
 * decodes/converts it as needed and pushes it to its sink.
 */
class AudioReceiver
    : public ReceiverBase, boost::noncopyable
{
    public:
        AudioReceiver(AudioSinkConfig aConfig, 
                      ReceiverConfig rConfig);

        ~AudioReceiver();

    private:
        void init_codec();
        void init_depayloader();
        void init_level();
        void init_sink();
        
        void setCaps(); 

        const AudioSinkConfig audioConfig_;
        const ReceiverConfig remoteConfig_;

        RtpReceiver session_;
        bool gotCaps_;
        RtpPay *depayloader_;
        Decoder *decoder_;
        AudioLevel level_;
        AudioSink *sink_;
};

#endif // _AUDIO_RECEIVER_H_

