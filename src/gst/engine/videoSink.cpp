/* videoSink.cpp
 * Copyright (C) 2008-2009 Société des arts technologiques (SAT)
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

#include "util.h"

#include <gst/interfaces/xoverlay.h>
#include "gstLinkable.h"
#include "videoSink.h"
#include "pipeline.h"
#include "rtpReceiver.h"
#include "gutil.h"


#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>
#include <X11/extensions/Xinerama.h>


void VideoSink::destroySink()
{
    pipeline_.remove(&sink_);
}

bool initializeGtk()
{
    gtk_init(0, NULL);
    return true;
}
        
GtkVideoSink::GtkVideoSink(Pipeline &pipeline, int screen_num) : 
    VideoSink(pipeline), 
    gtkInitialized_(initializeGtk()),
    window_(gtk_window_new(GTK_WINDOW_TOPLEVEL)), 
    screen_num_(screen_num), drawingArea_(gtk_drawing_area_new()),
	vbox_(gtk_vbox_new(FALSE, 0)),
	hbox_(gtk_hbox_new(FALSE, 0)),
	horizontalSlider_(0),
	sliderFrame_(0),
    isFullscreen_(false)
{
    gtk_box_pack_start(GTK_BOX(hbox_), vbox_, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(vbox_), drawingArea_, TRUE, TRUE, 0);

    gtk_container_add(GTK_CONTAINER(window_), hbox_);
    
    // add listener for window-state-event to detect fullscreenness
    g_signal_connect(G_OBJECT(window_), "window-state-event", G_CALLBACK(onWindowStateEvent), this);
}


gboolean GtkVideoSink::onWindowStateEvent(GtkWidget * /*widget*/, GdkEventWindowState *event, gpointer data)
{
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    context->isFullscreen_ = (event->new_window_state & GDK_WINDOW_STATE_FULLSCREEN);
    return TRUE;
}


Window GtkVideoSink::getXWindow()
{ 
    // FIXME: see https://bugzilla.gnome.org/show_bug.cgi?id=599885
    return GDK_WINDOW_XWINDOW(drawingArea_->window);
}


void GtkVideoSink::destroy_cb(GtkWidget * /*widget*/, gpointer data)
{

    LOG_DEBUG("Window closed, quitting.");
    GtkVideoSink *context = static_cast<GtkVideoSink*>(data);
    //context->pipeline_.quit();
    context->window_ = 0;
}


void GtkVideoSink::makeDrawingAreaBlack()
{
    GdkColor color;
    gdk_color_parse ("black", &color);
    gtk_widget_modify_bg(drawingArea_, GTK_STATE_NORMAL, &color);    // needed to ensure black background
}


void GtkVideoSink::showWindow()
{
    makeDrawingAreaBlack();
    gtk_widget_show_all(window_);
}


void GtkVideoSink::hideCursor()
{
    // FIXME: this is because gtk doesn't support GDK_BLANK_CURSOR before gtk-2.16
    char invisible_cursor_bits[] = { 0x0 };
    GdkCursor* cursor;
    GdkBitmap *empty_bitmap;
    GdkColor color = {0, 0, 0, 0};
    empty_bitmap = gdk_bitmap_create_from_data(GDK_WINDOW(drawingArea_->window),
            invisible_cursor_bits,
            1, 1);

    cursor = gdk_cursor_new_from_pixmap(empty_bitmap, empty_bitmap, &color,
            &color, 0, 0);

    gdk_window_set_cursor(GDK_WINDOW(drawingArea_->window), cursor);
}


void GtkVideoSink::toggleFullscreen(GtkWidget *widget)
{
    // toggle fullscreen state
    isFullscreen_ ? makeUnfullscreen(widget) : makeFullscreen(widget);
}


void GtkVideoSink::makeFullscreen(GtkWidget *widget)
{
    gtk_window_stick(GTK_WINDOW(widget));           // window is visible on all workspaces
    gtk_window_fullscreen(GTK_WINDOW(widget));
    if (horizontalSlider_)
        gtk_widget_hide(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_hide(sliderFrame_);
}


void GtkVideoSink::makeUnfullscreen(GtkWidget *widget)
{
    gtk_window_unstick(GTK_WINDOW(widget));           // window is not visible on all workspaces
    gtk_window_unfullscreen(GTK_WINDOW(widget));
    /// show controls
    if (horizontalSlider_)
        gtk_widget_show(horizontalSlider_);
    if (sliderFrame_)
        gtk_widget_show(sliderFrame_);
}


bool GtkVideoSink::handleMessage(const std::string &path, const std::string &arguments)
{
    if (path == "fullscreen")
    {
        toggleFullscreen();
        return true;
    }
    else if (path == "window-title")
    {
        gtk_window_set_title(GTK_WINDOW(window_), arguments.c_str());
        return true;
    }
    else if (path == "create-control")
    {
        createControl();
        return true;
    }

    return false;
}


/* makes the latency window */
void GtkVideoSink::createControl()
{
    LOG_INFO("Creating controls");
    sliderFrame_ = gtk_frame_new("Jitterbuffer Latency (ms)");
    // min, max, step-size
	horizontalSlider_ = gtk_hscale_new_with_range(RtpReceiver::MIN_LATENCY, RtpReceiver::MAX_LATENCY, 1.0);

    // set initial value
    gtk_range_set_value(GTK_RANGE(horizontalSlider_), RtpReceiver::INIT_LATENCY);

    gtk_container_add(GTK_CONTAINER(sliderFrame_), horizontalSlider_);
	gtk_box_pack_start(GTK_BOX(vbox_), sliderFrame_, FALSE, FALSE, 0);
    g_signal_connect(G_OBJECT(horizontalSlider_), "value-changed",
			 G_CALLBACK(RtpReceiver::updateLatencyCb), NULL);
    showWindow();
}

void VideoSink::prepareSink()
{
    //g_object_set(G_OBJECT(sink_), "sync", FALSE, NULL);
    g_object_set(G_OBJECT(sink_), "force-aspect-ratio", TRUE, NULL);
}


gboolean XvImageSink::key_press_event_cb(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    XvImageSink *context = static_cast<XvImageSink*>(data);
    switch (event->keyval)
    {
        case GDK_f:
        case GDK_F:
            context->toggleFullscreen(widget);
            break;

        case GDK_Q:
            // Quit application, this quits the main loop
            // (if there is one)
            LOG_INFO("Q key pressed, quitting.");
            context->VideoSink::pipeline_.quit();
            break;

        default:
            break;
    }

    return TRUE;
}


bool XvImageSink::handleBusMsg(GstMessage * message)
{
    // ignore anything but 'prepare-xwindow-id' element messages
    if (GST_MESSAGE_TYPE (message) != GST_MESSAGE_ELEMENT)
        return false;
 
    if (!gst_structure_has_name(message->structure, "prepare-xwindow-id"))
        return false;
 
    LOG_DEBUG("Got prepare-xwindow-id msg");
    gst_x_overlay_set_xwindow_id(GST_X_OVERLAY(GST_MESSAGE_SRC(message)), getXWindow());
  
    return true;
}

XvImageSink::XvImageSink(Pipeline &pipeline, int width, int height, int screenNum) : 
    GtkVideoSink(pipeline, screenNum),
    BusMsgHandler(pipeline)
{
    sink_ = VideoSink::pipeline_.makeElement("xvimagesink", NULL);
    prepareSink();

    /// FIXME: this is ifdef'd out to avoid getting that  Xinerama error msg every time

    LOG_DEBUG("Setting default window size to " << width << "x" << height);
    gtk_window_set_default_size(GTK_WINDOW(window_), width, height);
    //gtk_window_set_decorated(GTK_WINDOW(window_), FALSE);   // gets rid of border/title

    gtk_widget_set_events(window_, GDK_KEY_PRESS_MASK);
    g_signal_connect(G_OBJECT(window_), "key-press-event",
            G_CALLBACK(XvImageSink::key_press_event_cb), this);
    g_signal_connect(G_OBJECT(window_), "destroy",
            G_CALLBACK(destroy_cb), static_cast<gpointer>(this));

    showWindow();
    hideCursor();

    gtk_widget_set_size_request(drawingArea_, width, height);
}


XvImageSink::~XvImageSink()
{
    GtkVideoSink::destroySink();
    if (window_)
    {
        gtk_widget_destroy(window_);
        LOG_DEBUG("Videosink window destroyed");
    }
}


XImageSink::XImageSink(Pipeline &pipeline) : 
    VideoSink(pipeline),
    colorspc_(pipeline_.makeElement("ffmpegcolorspace", NULL)) 
{
    // ximagesink only supports rgb and not yuv colorspace, so we need a converter here
    sink_ = pipeline_.makeElement("ximagesink", NULL);
    prepareSink();

    gstlinkable::link(colorspc_, sink_);
}

XImageSink::~XImageSink()
{
    VideoSink::destroySink();
    pipeline_.remove(&colorspc_);
}

