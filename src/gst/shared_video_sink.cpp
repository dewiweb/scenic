/*
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 *
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include "shared_video_sink.h"
#include "gst_linkable.h"
#include "pipeline.h"
#include <gst/gst.h>

SharedVideoSink::SharedVideoSink(const Pipeline &pipeline, const std::string &sp) :
    VideoSink(),
    socketPath(sp)
{
    tee_ = pipeline.makeElement("tee", NULL);
    // fakesink in order to continue consuming buffers when no reader is present.
    // otherwise buffers accumulate in "tee" 
    GstElement *queue = pipeline.makeElement("queue",NULL);
    GstElement *fakesink = pipeline.makeElement("fakesink",NULL);
    writer_ = shmdata_base_writer_init (sp.c_str(), pipeline.getPipeline(), tee_);
    gstlinkable::link(tee_, queue);
    gstlinkable::link(queue, fakesink);
}

SharedVideoSink::~SharedVideoSink()
{
  shmdata_base_writer_close (writer_);
}

