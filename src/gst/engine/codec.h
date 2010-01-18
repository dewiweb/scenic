// codec.h
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

#ifndef _CODEC_H_
#define _CODEC_H_

#include "gstLinkable.h"

#include "noncopyable.h"

// forward declarations
class _GstElement;
class RtpPay;
class Pay;
class MapMsg;


/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc encoding of raw media streams.
 */
class Encoder : public GstLinkableFilter, boost::noncopyable
{
    public:
        Encoder();
        virtual ~Encoder();
        /// Abstract Factory method that will create payloaders corresponding to this Encoder's codec type 
        virtual Pay* createPayloader() const = 0;
        int getBitrate() const;
        virtual void setBitrate(int bitrate);

    protected:
        virtual void setBitrateInKbs(int bitrate);
        _GstElement *encoder_;

    private:
        _GstElement *srcElement() { return encoder_; }
        _GstElement *sinkElement() { return encoder_; }
};

/** 
 *  Abstract child of Codec that wraps a single GstElement, and which exposes both a source and sink 
 *  and whose concrete subclasses will provide specifc decoding of encoded media streams.
 */
class Decoder : public GstLinkableFilter, boost::noncopyable
{
    public:
        Decoder();
        virtual ~Decoder();
        /// Abstract Factory method that will create depayloaders corresponding to this Decoder's codec type 
        virtual RtpPay* createDepayloader() const = 0;
        virtual void adjustJitterBuffer() {}; // buy default, do nothing
        virtual bool adjustsBufferTime() { return false; }
        virtual unsigned long long minimumBufferTime() { THROW_ERROR("Unimplemented"); return 0; }
        
    protected:
        _GstElement *decoder_;

    private:
        _GstElement *srcElement() { return decoder_; }
        _GstElement *sinkElement() { return decoder_; }
};


class VideoEncoder : public Encoder 
{
    public: 
        VideoEncoder(GstElement *encoder, bool supportsInterlaced);
        ~VideoEncoder();

    protected:
        _GstElement *colorspc_;
        bool supportsInterlaced_;

    private:
        
        _GstElement *sinkElement() 
        { 
            return colorspc_;
        }
};


class VideoDecoder : public Decoder 
{
    public: 
        VideoDecoder();
        ~VideoDecoder();
        void doDeinterlace() { doDeinterlace_ = true; }
        virtual void init() = 0;
        virtual void adjustJitterBuffer();
    
    protected:
        bool doDeinterlace_;
        _GstElement *colorspc_;
        _GstElement *deinterlace_;
        //_GstElement *queue_;
        static const unsigned long long LONGER_JITTER_BUFFER_MS = 60;

    private:
        const static int MAX_QUEUE_BUFFERS = 3;
        
        _GstElement *srcElement() 
        { 
            // return queue_;
            if (!doDeinterlace_)
                return decoder_;
            else 
                return deinterlace_;
        }
};

/// Encoder that encodes raw video into H.264 using the x264 encoder
class H264Encoder : public VideoEncoder
{
    public: 
        H264Encoder(MapMsg &settings);
        void setBitrate(int bitrate);

    private:
        ~H264Encoder();
        Pay* createPayloader() const;
        int bitrate_;
};

/// Decoder that decodes H.264 into raw video using the ffdec_h264 decoder.
class H264Decoder : public VideoDecoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
        void adjustJitterBuffer(); 
};



/// Encoder that encodes raw video into H.263 using the ffmpeg h263 encoder
class H263Encoder : public VideoEncoder
{
    public: 
        H263Encoder(MapMsg &settings);

    private:
        int bitrate_;
        ~H263Encoder();

        
        Pay* createPayloader() const;
};

/// Decoder that decodes H.263 into raw video using the ffmpeg h263 decoder.
class H263Decoder : public VideoDecoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};



/// Encoder that encodes raw video into mpeg4 using the ffmpeg mpeg4 encoder
class Mpeg4Encoder : public VideoEncoder
{
    public:
        Mpeg4Encoder(MapMsg &settings);
        ~Mpeg4Encoder();

    private:
        int bitrate_;
        Pay* createPayloader() const;
};


/// Decoder that decodes mpeg4 into raw video using the ffmpeg mpeg4 decoder.
class Mpeg4Decoder: public VideoDecoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw video into theora using the theoraenc encoder
class TheoraEncoder : public VideoEncoder
{
    public:
        TheoraEncoder(MapMsg &settings);
        ~TheoraEncoder();
        void setBitrate(int bitrate);
        void setQuality(int quality);
        void setSpeedLevel(int speedLevel);

    private:
        static const int MAX_SPEED_LEVEL = 2;
        static const int MIN_SPEED_LEVEL = 0;
        static const int MIN_QUALITY = 0;
        static const int MAX_QUALITY = 63;  // defined in plugin
        static const int INIT_QUALITY = 20;
        Pay* createPayloader() const;
        int bitrate_;
        int quality_;
};


/// Decoder that decodes mpeg4 into raw video using the theoradec decoder.
class TheoraDecoder: public VideoDecoder
{
    private: 
        void init();
        RtpPay* createDepayloader() const;
};


/// Encoder that encodes raw audio using the vorbis encoder.
class VorbisEncoder : public  Encoder
{
    public: 
        VorbisEncoder();

    private:
        ~VorbisEncoder();
        Pay* createPayloader() const;
};

/// Decoder that decodes vorbis into raw audio using the vorbis decoder.
class VorbisDecoder : public Decoder
{
    public: 
        VorbisDecoder();
        bool adjustsBufferTime() { return true; }
        unsigned long long minimumBufferTime();
    private: 
        RtpPay* createDepayloader() const;
        static const unsigned long long MIN_BUFFER_USEC = 100000;
};

/// Encoder that simply performs datatype conversion on raw audio.
class RawEncoder : public Encoder
{
    public:
        RawEncoder();
        ~RawEncoder();
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return aconv_; }

    private:
        _GstElement *aconv_;
        Pay* createPayloader() const;
};

/// Decoder that simply performs datatype conversion on raw audio.
class RawDecoder : public Decoder
{
    public:
        RawDecoder();
        ~RawDecoder();

    private:
        RtpPay* createDepayloader() const;
        _GstElement *aconv_;

        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return aconv_; }
};


/// Encoder that encodes raw audio to mpeg.
class LameEncoder : public Encoder
{
    public:
        LameEncoder();
        ~LameEncoder();

    private:
        _GstElement *aconv_;
        _GstElement *mp3parse_;
        Pay* createPayloader() const;
        _GstElement *sinkElement() { return aconv_; }
        _GstElement *srcElement() { return mp3parse_; }
        
        ///No Copy Constructor
        LameEncoder(const LameEncoder&);     
        ///No Assignment Operator
        LameEncoder& operator=(const LameEncoder&);     
};

/// Decoder that decodes mpeg to raw audio.

class MadDecoder : public Decoder
{
    public:
        MadDecoder();
        ~MadDecoder();
    private:
        _GstElement *srcElement() { return aconv_; }
        _GstElement *aconv_;
        void init();
        RtpPay* createDepayloader() const;
};

#endif //_CODEC_H_

