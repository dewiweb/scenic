//
// Copyright (C) 2008-2009 Société des arts technologiques (SAT)
// http://www.sat.qc.ca
// All rights reserved.
//
// This file is part of Scenic.
//
// Scenic is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Scenic is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with Scenic.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef _SHARED_VIDEO_SINK_H_
#define _SHARED_VIDEO_SINK_H_

#include "video_sink.h"
#include <shmdata/base-writer.h>

class _GtkWidget;
class _GstElement;
//class SharedVideoBuffer;

class SharedVideoSink : public VideoSink
{
    public:
        SharedVideoSink(const Pipeline &pipeline, const std::string& socketPath);
        virtual ~SharedVideoSink();

    private:
        const std::string socketPath;
        _GstElement *tee_;
	shmdata_base_writer_t *writer_;
        virtual _GstElement *sinkElement()
        {
            return tee_;
        }
};

#endif  // _SHARED_VIDEO_SINK_H_

