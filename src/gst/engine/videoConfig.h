/* videoConfig.h
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
#ifndef _VIDEO_CONFIG_H_
#define _VIDEO_CONFIG_H_

#include <string>

class MapMsg;
class Pipeline;
class VideoSource;
class VideoScale;
class VideoFlip;
class VideoSink;

class VideoSourceConfig
{
    public:
        VideoSourceConfig(MapMsg &msg);
        VideoSource* createSource(Pipeline &pipeline) const;  // factory method

        const char *source() const { return source_.c_str(); }
        std::string sourceString() const { return source_; }
        static int listCameras();
        int bitrate() const { return bitrate_; }
        int quality() const { return quality_; }

        bool hasLocation() const { return !location_.empty(); }
        bool hasDeviceName() const { return !deviceName_.empty(); }
        bool hasCameraNumber() const { return cameraNumber_ != -1; }
        int cameraNumber() const { return cameraNumber_; }
        unsigned long long GUID() const { return GUID_; }
        bool hasGUID() const { return GUID_ != 0; }
        int framerate() const { return framerate_; }
        bool locationExists() const;
        bool deviceExists() const;
        const char *location() const;
        const char *deviceName() const;
        unsigned captureWidth() const;
        unsigned captureHeight() const;
        std::string pictureAspectRatio() const;
        bool forceGrayscale() const;
        std::string pixelAspectRatio() const;
        /// used by other classes, that's why it's not a member function
        static std::string calculatePixelAspectRatio(int width, int height, 
                const std::string &pictureAspectRatio);
        static void setStandard(const std::string &videoDevice, std::string videoStandard);

    private:
        const std::string source_;
        const int bitrate_;
        const int quality_;
        const std::string deviceName_;
        const std::string location_;
        const int cameraNumber_;
        const unsigned long long GUID_;
        const int framerate_;
        const int captureWidth_;
        const int captureHeight_;
        const bool grayscale_;
        const std::string pictureAspectRatio_;
};


class VideoSinkConfig 
{
    public:

        VideoSinkConfig(MapMsg &msg);
        VideoSink* createSink(Pipeline &pipeline) const;
        VideoScale* createVideoScale(Pipeline &pipeline) const;
        VideoFlip* createVideoFlip(Pipeline &pipeline) const;
        bool doDeinterlace() const { return doDeinterlace_; }

    private:
        bool resolutionIsInverted() const;
        int effectiveDisplayWidth() const;
        int effectiveDisplayHeight() const;
        const std::string sink_;
        const int screenNum_;
        bool doDeinterlace_;
        const std::string sharedVideoId_;
        const int displayWidth_;
        const int displayHeight_;
        const std::string flipMethod_;
};

#endif // _VIDEO_CONFIG_H_

