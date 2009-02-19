// audioSource.h
// Copyright (C) 2009 Société des arts technologiques (SAT)
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

/** \file audioSource.h
 *      A family of classes representing different types/sources of audio input.
 */

#ifndef _AUDIO_SOURCE_H_
#define _AUDIO_SOURCE_H_

#include "gstLinkable.h"
#include "interleave.h"
#include "busMsgHandler.h"

// forward declarations
class AudioSourceConfig;

/** 
 *  Abstract base class from which our audio sources are derived.
 *  Uses template method to define the initialization process that subclasses will have to
 *  implement (in part) and/or override. Any direct, concrete descendant of this class will not
 *  need to have its audio frames interleaved.
 */

class AudioSource : public GstLinkableSource
{
    public:
        ~AudioSource();
        virtual void init();

    protected:
        explicit AudioSource(const AudioSourceConfig &config);
        
        /// Implemented by subclasses to perform other specific initialization 
        virtual void sub_init() = 0;

        /// Audio parameter object 
        const AudioSourceConfig &config_;
        
        /// GstElements representing each source and audioconvert 
        GstElement *source_;

    private:
        
        GstElement *srcElement() { return source_; }
        
        /// No Copy Constructor
        AudioSource(const AudioSource&);     
        /// No Assignment Operator
        AudioSource& operator=(const AudioSource&);     
};

/** 
 * Abstract child of AudioSource whose audio frames must be interleaved before pushing audio further down pipeline.
 */

class InterleavedAudioSource
    : public AudioSource
{
    protected:
        /// Object initializer 
        void sub_init();

        explicit InterleavedAudioSource(const AudioSourceConfig &config);
        
        ~InterleavedAudioSource();

        /// Object which performs the interleaving of this source's channels 
        Interleave interleave_;
        std::vector<GstElement*> sources_, aconvs_;

    private:
        
        GstElement *srcElement() { return interleave_.srcElement(); }
};

/** 
 *  Concrete InterleavedAudioSource which gives us an array of sine-wave generating sources.
 *
 *  AudioTestSource generates sine-tones, each of which alternate between two hard-coded frequencies. Their
 *  combined output is then interleaved.
 */

class AudioTestSource : public InterleavedAudioSource
{
    public:
        explicit AudioTestSource(const AudioSourceConfig &config);
        
    private:
        void sub_init();

        ~AudioTestSource();

        static gboolean timedCallback(GstClock *clock, 
                                      GstClockTime time, 
                                      GstClockID id,
                                      gpointer user_data);
        void toggle_frequency();

        GstClockID clockId_;
        int offset_;

        static const double FREQUENCY[2][8];
        /// No Copy Constructor
        AudioTestSource(const AudioTestSource&);     
        /// No Assignment Operator
        AudioTestSource& operator=(const AudioTestSource&);     
};

/** 
 *  \class AudioFileSource
 *  Concrete AudioSource which provides playback of files. 
 *
 *  AudioFileSource plays back a file (determined by its AudioSourceConfig object). Depending
 *  on its AudioSourceConfig object, it may loop the file. It implements the BusMsgHandler interface
 *  so that it knows when an End-of-Signal event occurs, in which case it may
 *  play the file again if it has been set to continue to play the file (either infinitely or for a finite
 *  number of plays).
 */

class AudioFileSource : public AudioSource, public BusMsgHandler
{
    public:
        explicit AudioFileSource(const AudioSourceConfig &config);

    private:
        ~AudioFileSource();
         
        bool handleBusMsg(_GstMessage *msg);
        
        static void cb_new_src_pad(GstElement * srcElement, 
                                   GstPad * srcPad, 
                                   gboolean last,
                                   void *data);
        void loop(int nTimes);
        void sub_init();

        void restartPlayback();
        GstElement* decoder_;
        GstElement* aconv_;
        int loopCount_;
        static const int LOOP_INFINITE;
        
        /// No Copy Constructor
        AudioFileSource(const AudioFileSource&);     
        /// No Assignment Operator
        AudioFileSource& operator=(const AudioFileSource&);     
};

/** 
 *  Concrete AudioSource which captures audio from ALSA
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioAlsaSource : public AudioSource
{
    public:
        explicit AudioAlsaSource(const AudioSourceConfig &config);

    private:
        ~AudioAlsaSource();

        void sub_init();
        GstElement *srcElement() { return capsFilter_; }

        GstElement *capsFilter_;
        GstElement *aconv_;
        /// No Copy Constructor
        AudioAlsaSource(const AudioAlsaSource&);     
        /// No Assignment Operator
        AudioAlsaSource& operator=(const AudioAlsaSource&);     
};

/** 
 *  Concrete AudioSource which captures audio from PulseAudio
 *  Has caps filter to allow number of channels to be variable.
 */

class AudioPulseSource : public AudioSource
{
    public:
        explicit AudioPulseSource(const AudioSourceConfig &config);
    private:
        ~AudioPulseSource();
    
        void sub_init();
        GstElement *srcElement() { return capsFilter_; }
    
        GstElement *capsFilter_;
        GstElement *aconv_;
        /// No Copy Constructor
        AudioPulseSource(const AudioPulseSource&);     
        /// No Assignment Operator
        AudioPulseSource& operator=(const AudioPulseSource&);     
};

/** 
 *  Concrete AudioSource which captures audio from JACK, 
 *  and interleaves incoming jack buffers into one multichannel stream.
 */

class AudioJackSource : public AudioSource
{
    public:
        explicit AudioJackSource(const AudioSourceConfig &config);
    
    private:
        ~AudioJackSource();

        GstElement *srcElement() { return capsFilter_; }
        void sub_init();

        GstElement *capsFilter_;
        GstElement *aconv_;
        /// No Copy Constructor
        AudioJackSource(const AudioJackSource&);     
        /// No Assignment Operator
        AudioJackSource& operator=(const AudioJackSource&);     
};


/** 
 *  Concrete AudioSource which captures audio from dv device.
 *
 *  This object is tightly coupled with VideoDvSource, as both (if present) will share one source_ GstElement, 
 *  and one dvdemux GstElement. It will look for both of these in the pipeline before trying to instantiate them. 
 *  If these GstElement are already present, AudioDvSource will simply store their addresses and link to them, if not
 *  it will create them. DVAudio can only be stereo or 4 channels according to gstreamer.
 */

class AudioDvSource : public AudioSource
{
    public:
        explicit AudioDvSource(const AudioSourceConfig &config);
    
    private:
        ~AudioDvSource();
        void sub_init();

        GstElement *queue_;
        GstElement *srcElement() { return queue_; }
        /// No Copy Constructor
        AudioDvSource(const AudioDvSource&);     
        /// No Assignment Operator
        AudioDvSource& operator=(const AudioDvSource&);     
};

#endif //_AUDIO_SOURCE_H_


