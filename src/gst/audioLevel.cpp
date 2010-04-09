/* audioLevel.cpp
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

#include "util.h"

#include <cstring>
#include <cmath>
#include <gst/gst.h>
#include "gutil.h"
#include "audioLevel.h"
#include "vumeter.h"
#include "pipeline.h"

#include <iterator>


/** Constructor sets by default emitMessages to true 
 * and message interval to one second */
AudioLevel::AudioLevel(Pipeline &pipeline, int numChannels, GdkNativeWindow socketID) : 
    BusMsgHandler(&pipeline),
    pipeline_(pipeline),
    level_(pipeline_.makeElement("level", NULL)),
    emitMessages_(true)
{
    static const int SPACING = 1;
    GtkWidget *hbox = gtk_hbox_new(FALSE/*homogenous spacing*/, SPACING);

    for (int i = 0; i < numChannels; ++i)
    {
        vumeters_.push_back(gtk_vumeter_new());
        gtk_box_pack_start(GTK_BOX(hbox), vumeters_[i], FALSE, FALSE, 0);
    }

    /* make window */
    GtkWidget *plug = gtk_plug_new(socketID);
    /* end main loop when plug is destroyed */
    g_signal_connect(G_OBJECT (plug), "destroy", G_CALLBACK(gutil::killMainLoop), NULL);
    GtkWidget *scrolled = gtk_scrolled_window_new(0, 0);
    g_object_set(scrolled, "vscrollbar-policy", GTK_POLICY_NEVER, NULL);
    g_object_set(scrolled, "hscrollbar-policy", GTK_POLICY_AUTOMATIC, NULL);
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), hbox);
    gtk_container_add(GTK_CONTAINER (plug), scrolled);
    /* show window and log its id */
    gtk_widget_show_all(plug);
    LOG_DEBUG("Created plug with ID: " << static_cast<unsigned int>(gtk_plug_get_id(GTK_PLUG(plug))));

    g_object_set(G_OBJECT(level_), "message", emitMessages_, NULL);
    static const guint64 INTERVAL_NS = 75000000;
    static const gdouble PEAK_FALLOFF = 320.0;
    static const guint64 PEAK_TTL = 3 * 300000000;  // 3 times default
    g_object_set(G_OBJECT(level_), "interval", INTERVAL_NS, NULL);
    g_object_set(G_OBJECT(level_), "peak-falloff", PEAK_FALLOFF, NULL);
    g_object_set(G_OBJECT(level_), "peak-ttl", PEAK_TTL, NULL);
}

/// Destructor 
AudioLevel::~AudioLevel()
{
    pipeline_.remove(&level_);
}

/**
 * Toggles whether or not this AudioLevel will post messages on the bus. */
void AudioLevel::emitMessages(bool doEmit)
{
    emitMessages_ = doEmit;
    g_object_set(G_OBJECT(level_), "message", emitMessages_, NULL);
}


/// Converts from decibel to linear (0.0 to 1.0) scale. 
double AudioLevel::dbToLinear(double db)
{
    return pow(10, db * 0.05);
}

void
AudioLevel::setValue(gdouble peak, gdouble decayPeak, GtkWidget *vumeter)
{
  GdkRegion *region;

  GTK_VUMETER(vumeter)->peak = peak;
  GTK_VUMETER(vumeter)->decay_peak = decayPeak;

  region = gdk_drawable_get_clip_region (vumeter->window);
  gdk_window_invalidate_region (vumeter->window, region, TRUE);
  gdk_window_process_updates (vumeter->window, TRUE);
}

/** 
 * The level message is posted on the bus by the level element, 
 * received by this AudioLevel, and dispatched. */
bool AudioLevel::handleBusMsg(GstMessage *msg)
{
    const GstStructure *s = gst_message_get_structure(msg);
    const gchar *name = gst_structure_get_name(s);
    std::vector<double> rmsValues;
    const std::string levelStr = "level";

    if (std::string(name) == levelStr) {   // this is level's msg
        guint channels;
        double peak_db;
        double decay_db;
        const GValue *list;
        const GValue *value;

        // we can get the number of channels as the length of the value list
        list = gst_structure_get_value (s, "rms");
        channels = gst_value_list_get_size (list);

        for (size_t c = 0; c < channels; ++c) {
            list = gst_structure_get_value(s, "peak");
            value = gst_value_list_get_value(list, c);
            peak_db = g_value_get_double(value);
            list = gst_structure_get_value (s, "decay");
            value = gst_value_list_get_value(list, c);
            decay_db = g_value_get_double(value);
            setValue(peak_db, decay_db, vumeters_[c]);
        }

        return true;
    }

    return false;           // this wasn't our msg, someone else should handle it
}


/// Prints current rms values through the LogWriter system. 
void AudioLevel::print(const std::vector<double> &rmsValues) const
{
    std::ostringstream os;
    std::copy(rmsValues.begin(), rmsValues.end(), std::ostream_iterator<double>(os, " "));

    LOG_DEBUG("rms values: " << os.str());
}

/// Sets the reporting interval in nanoseconds. 
void AudioLevel::interval(unsigned long long newInterval)
{
    tassert(newInterval > 0);
    g_object_set (G_OBJECT(level_), "interval", newInterval, "message", emitMessages_, NULL);
}

