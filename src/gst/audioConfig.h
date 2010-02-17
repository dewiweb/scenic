
/* audioConfig.h
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

#ifndef _AUDIO_LOCAL_CONFIG_H_
#define _AUDIO_LOCAL_CONFIG_H_

#include <string>

// forward declarations
class Pipeline;
class AudioSource;
class AudioSink;
class MapMsg;

/// Immutable class that is used to parameterize AudioSender objects. 
class AudioSourceConfig
{
    public:
        
        AudioSourceConfig(MapMsg &msg);     
        
        const char *source() const;

        int numChannels() const;

        bool hasDeviceName() const { return !deviceName_.empty(); }
        bool hasLocation() const { return !location_.empty(); }

        const char *sourceName() const;
        const char *deviceName() const;
        const char *location() const;

        bool locationExists() const;
         
        AudioSource* createSource(Pipeline &pipeline) const;

    private:
        const std::string source_;
        const std::string sourceName_;
        const std::string deviceName_;
        const std::string location_;
        const int numChannels_;
};

///  Immutable class that is used to parametrize AudioReceiver objects.  
class AudioSinkConfig
{
    public:
        AudioSinkConfig(MapMsg &msg);
        
        AudioSink* createSink(Pipeline &pipeline) const;
        bool hasDeviceName() const { return !deviceName_.empty(); }
        const char *sinkName() const;
        const char *deviceName() const;
        unsigned long long bufferTime() const;
        static const unsigned long long DEFAULT_BUFFER_TIME = 11333LL;

    private:
        const std::string sink_;
        const std::string sinkName_;
        const std::string deviceName_;
        const unsigned long long bufferTime_;
};

#endif // _AUDIO_LOCAL_CONFIG_H_
