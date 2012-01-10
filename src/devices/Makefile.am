noinst_LTLIBRARIES = libdevices.la

libdevices_la_CPPFLAGS = \
    $(GLIB_CFLAGS) \
    $(JACK_CFLAGS) \
    $(LIBDC1394_CFLAGS) \
    $(LIBRAW1394_CFLAGS) \
   -I$(top_srcdir)/src \
   -I$(top_srcdir)/src/include

libdevices_la_SOURCES = \
    dc1394.cpp \
    raw1394_util.cpp \
    v4l2_util.cpp

EXTRA_DIST = \
    dc1394.h \
    raw1394_util.h \
    v4l2_util.h

libdevices_la_LIBADD = \
    $(JACK_LIBS) \
    $(LIBDC1394_LIBS) \
    $(LIBRAW1394_LIBS)

