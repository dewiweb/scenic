SUBDIRS = test

noinst_LTLIBRARIES = libvumeter.la

libvumeter_la_SOURCES = \
    vumeter.cpp \
    $(top_srcdir)/src/include/vumeter.h

INCLUDES = \
    -I$(top_srcdir)/src/include

libvumeter_la_CPPFLAGS = \
    $(CAIRO_CFLAGS) \
    $(GLIB_CFLAGS) \
    $(GTK_CFLAGS)

libvumeter_la_LIBADD = \
    $(CAIRO_LIBS) \
    $(GLIB_LIBS) \
    $(GTK_LIBS)

