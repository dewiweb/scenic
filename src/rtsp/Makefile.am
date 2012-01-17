SUBDIRS = examples

noinst_LTLIBRARIES = librtsp.la
librtsp_la_SOURCES = \
    gst-rtsp-cam-media-factory.c \
    rtsp_server.cpp \
    rtsp_client.cpp

EXTRA_DIST = \
    gst-rtsp-cam-media-factory.h \
    rtsp_server.h \
    rtsp_client.h

AM_CXXFLAGS = \
    $(GST_CFLAGS) \
    $(GST_RTSP_SERVER_CFLAGS) \
    $(GTK_CFLAGS) \
    -I$(top_srcdir)/src \
    -I$(top_srcdir)/src/include

AM_CFLAGS = \
   $(GLIB_CFLAGS) \
   $(GST_CFLAGS) \
   $(GST_RTSP_CFLAGS)

librtsp_la_LIBADD = \
    $(GLIB_LIBS) \
    $(GST_LIBS) \
    $(GST_RTSP_SERVER_LIBS) \
    $(GTK_LIBS) \
    $(top_builddir)/src/devices/libdevices.la

