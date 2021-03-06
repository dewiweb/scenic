/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 * http://www.sat.qc.ca
 * All rights reserved.
 *
 * This file is part of Scenic.
 *
 * Scenic is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Scenic is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <boost/program_options.hpp>
#include "util/log_writer.h"

#include "audio_config.h"
#include "audio_source.h"
#include "audio_level.h"
#include "audio_sink.h"
#include "jack_util.h"
#include "pipeline.h"

static const int USEC_PER_MILLISEC = 1000;
namespace po = boost::program_options;

AudioSourceConfig::AudioSourceConfig(const po::variables_map &options) :
    source_(options["audiosource"].as<std::string>()),
    bitrate_(options["audiobitrate"].as<int>()),
    quality_(options["audioquality"].as<double>()),
    sourceName_(options["jack-client-name"].as<std::string>()),
    deviceName_(options["audiodevice"].as<std::string>()),
    location_(options["audiolocation"].as<std::string>()),
    numChannels_(options["numchannels"].as<int>()),
    bufferTime_(options["audio-buffer"].as<int>() * USEC_PER_MILLISEC),
    socketID_(options["vumeter-id"].as<unsigned long>()),
    disableAutoConnect_(options["disable-jack-autoconnect"].as<bool>())
{
    using boost::lexical_cast;
    using std::string;
    if (numChannels_ < 1)
        throw std::range_error("Invalid number of channels=" +
                lexical_cast<string>(numChannels_));
}

int AudioSourceConfig::bitrate() const
{
    return bitrate_;
}


double AudioSourceConfig::quality() const
{
    return quality_;
}

/// Returns c-style string specifying the source
const char *AudioSourceConfig::source() const
{
    return source_.c_str();
}

/// Returns number of channels
int AudioSourceConfig::numChannels() const
{
    return numChannels_;
}

/// Returns buffer time, which must be an unsigned long long for gstreamer's audiosink to accept it safely
unsigned long long AudioSourceConfig::bufferTime() const
{
    return bufferTime_;
}

/// Factory method that creates an AudioSource based on this object's source_ string
AudioSource* AudioSourceConfig::createSource(Pipeline &pipeline) const
{
    if (source_ == "audiotestsrc")
        return new AudioTestSource(pipeline, *this);
    else if (source_ == "filesrc")
        return new AudioFileSource(pipeline, *this);
    else if (source_ == "alsasrc" or source_ == "pulsesrc" or
            source_ == "autoaudiosrc" or source_ == "gconfaudiosrc")
        return new AudioSimpleSource(pipeline, *this);
    else if (source_ == "jackaudiosrc")
    {
        jack::assertReady();
        AudioJackSource *result = new AudioJackSource(pipeline, *this);
        if (disableAutoConnect_)
            result->disableAutoConnect();
        return result;
    }
    else if (source_ == "dv1394src")
        return new AudioDvSource(pipeline, *this);
    else
        THROW_ERROR(source_ << " is an invalid audiosource");
    return 0;
}

/// Factory method that creates an AudioLevel based on this object's socketID
AudioLevel* AudioSourceConfig::createLevel(Pipeline &pipeline, const std::string &title) const
{
    return new AudioLevel(pipeline, numChannels_, socketID_, title);
}

/// Fixme: abstract the common stuff into baseclass
/// Factory method that creates an AudioLevel based on this object's socketID
AudioLevel* AudioSinkConfig::createLevel(Pipeline &pipeline, const std::string &title) const
{
    return new AudioLevel(pipeline, numChannels_, socketID_, title);
}

/// Returns c-style string specifying the location (filename)
const char* AudioSourceConfig::location() const
{
    return location_.c_str();
}


/// Returns c-style string specifying the device (i.e. plughw:0)
const char* AudioSourceConfig::deviceName() const
{
    return deviceName_.c_str();
}

/// Returns c-style string specifying the source name
const char* AudioSourceConfig::sourceName() const
{
    if (sourceName_ != "")
        return sourceName_.c_str();
    else
        return 0;
}


/// Returns true if location indicates an existing, readable file/device.
bool AudioSourceConfig::locationExists() const
{
    return g_file_test(location_.c_str(), G_FILE_TEST_EXISTS);
}


/// Constructor
AudioSinkConfig::AudioSinkConfig(const po::variables_map &options) :
    sink_(options["audiosink"].as<std::string>()),
    sinkName_(options["jack-client-name"].as<std::string>()),
    deviceName_(options["audiodevice"].as<std::string>()),
    bufferTime_(options["audio-buffer"].as<int>() * USEC_PER_MILLISEC),
    socketID_(options["vumeter-id"].as<unsigned long>()),
    numChannels_(options["numchannels"].as<int>()),
    disableAutoConnect_(options["disable-jack-autoconnect"].as<bool>())
{}

/// Factory method that creates an AudioSink based on this object's sink_ string
AudioSink* AudioSinkConfig::createSink(Pipeline &pipeline) const
{
    if (sink_ == "jackaudiosink")
    {
        jack::assertReady();
        AudioJackSink * result = new AudioJackSink(pipeline, *this);
        if (disableAutoConnect_)
            result->disableAutoConnect();
        return result;
    }
    else if (sink_ == "alsasink" or sink_ == "pulsesink" or sink_ ==
            "autoaudiosink" or sink_ == "gconfaudiosink")
        return new AudioSimpleSink(pipeline, *this);
    else
    {
        THROW_CRITICAL(sink_ << " is an invalid audiosink");
        return 0;
    }
}

/// Returns c-style string specifying the source name
const char* AudioSinkConfig::sinkName() const
{
    if (sinkName_ != "")
        return sinkName_.c_str();
    else
        return 0;
}


/// Returns c-style string specifying the device used
const char* AudioSinkConfig::deviceName() const
{
    return deviceName_.c_str();
}

int AudioSinkConfig::numChannels() const
{
    return numChannels_;
}

/// Returns buffer time, which must be an unsigned long long for gstreamer's audiosink to accept it safely
unsigned long long AudioSinkConfig::bufferTime() const
{
    return bufferTime_;
}

/// Returns c-style string specifying the source
const char *AudioSinkConfig::sink() const
{
    return sink_.c_str();
}

