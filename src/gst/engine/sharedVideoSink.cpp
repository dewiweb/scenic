/* 
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
 *
 * http://www.sat.qc.ca
 * All rights reserved.
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

#include "./sharedVideoSink.h"
#include "./sharedVideoBuffer.h"
#include "util.h"

#include "./gstLinkable.h"
#include "./pipeline.h"
#include <gst/app/gstappbuffer.h>
#include <gst/app/gstappsink.h>

#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>


using boost::interprocess::shared_memory_object;
using boost::interprocess::read_write;
using std::tr1::shared_ptr;


const std::string SharedVideoSink::id_("shared_memory");


shared_ptr<shared_memory_object> SharedVideoSink::createSharedMemory()
{
    using boost::interprocess::create_only;

    removeSharedMemory();
    // create a shared memory object
    shared_ptr<shared_memory_object> shm(new shared_memory_object(create_only, id_.c_str(), read_write)); 
    // set size
    shm->truncate(sizeof(SharedVideoBuffer));
    return shm;
}


bool SharedVideoSink::removeSharedMemory()
{
    // Erase previously shared memory
    return shared_memory_object::remove(id_.c_str());
}


SharedVideoSink::SharedVideoSink() : 
    colorspc_(0), 
    shm_(createSharedMemory()), 
    region_(*shm_, read_write), // map the whole shared memory in this process
    sharedBuffer_(0)
{
    // get the address of the mapped region
    void *addr = region_.get_address();

    // construct the shared structure in memory with placement new
    sharedBuffer_ = new (addr) SharedVideoBuffer;
}


void SharedVideoSink::onNewBuffer(GstElement *elt, SharedVideoSink *context)
{
    using boost::interprocess::scoped_lock;
    using boost::interprocess::interprocess_mutex;
    using boost::interprocess::interprocess_exception;

    static unsigned long long bufferCount = 0;
    GstBuffer *buffer = 0;
    guint size;

    try
    {
        /* get the buffer from appsink */
        buffer = gst_app_sink_pull_buffer(GST_APP_SINK(elt));

        // lock the mutex
        scoped_lock<interprocess_mutex> lock(context->sharedBuffer_->getMutex());
        // if a buffer has been pushed, wait until the consumer tells us
        // it's consumed it
        context->sharedBuffer_->waitOnConsumer(lock);

        if (context->sharedBuffer_->hasSentinel())
        {
            g_print("Pushed %lld buffers, should be quitting.\n", bufferCount);
        }
        else    
        {
            std::cout << "PUSHING THE BUFFER\n";
            // push the buffer
            size = GST_BUFFER_SIZE (buffer);
            context->sharedBuffer_->pushBuffer(GST_BUFFER_DATA(buffer), size);
            ++bufferCount;
        }

        context->sharedBuffer_->notifyConsumer();
        // mutex is released here (goes out of scope)
    }
    catch (interprocess_exception &ex)
    {
        removeSharedMemory();
        g_print("%s\n", ex.what());
        /* we don't need the appsink buffer anymore */
        gst_buffer_unref(buffer);
        return;
    }

    /* we don't need the appsink buffer anymore */
    gst_buffer_unref(buffer);
}


void SharedVideoSink::prepareSink()
{
    // FIXME: fixed caps are lame
    GstCaps *videoCaps = gst_caps_from_string("video/x-raw-rgb, bpp=16, depth=16, width=640, height=480");
    g_object_set(G_OBJECT(sink_), "emit-signals", TRUE, "caps", videoCaps, NULL);
    g_signal_connect(sink_, "new-buffer", G_CALLBACK(onNewBuffer), this);
    gst_caps_unref(videoCaps);
}


void SharedVideoSink::init()
{
    colorspc_ = Pipeline::Instance()->makeElement("ffmpegcolorspace", NULL);
    sink_ = Pipeline::Instance()->makeElement("appsink", NULL);
    gstlinkable::link(colorspc_, sink_);
    prepareSink();
}


SharedVideoSink::~SharedVideoSink()
{
    destroySink();
    Pipeline::Instance()->remove(&colorspc_);
    shared_memory_object::remove(id_.c_str());
}


void SharedVideoSink::handleMessage(const std::string &message)
{
    VideoSink::defaultMessage(message);
}
