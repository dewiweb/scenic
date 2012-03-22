SUBDIRS = test

bin_PROGRAMS = milhouse

milhouse_SOURCES = \
    $(top_srcdir)/src/include/concurrent_queue.h \
    main.cpp \
    milhouse.cpp \
    milhouse_logger.cpp \
    program_options.cpp

EXTRA_DIST = \
    milhouse_logger.h \
    milhouse.h \
    program_options.h


#FIXME GLIB_CFLAGS only here because pipeline.h has include glib/gtypes.h, also GDK flags because of audioSender.h->audioLevel.h

milhouse_CPPFLAGS = \
    $(GLIB_CFLAGS) \
    -I$(top_srcdir)/src/include \
    -I$(top_srcdir)/src

milhouse_LDADD = \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_LDFLAGS) \
    $(BOOST_PROGRAM_OPTIONS_LIB) \
    $(BOOST_SYSTEM_LIB) \
    $(BOOST_THREAD_LIB) \
    $(GLU_LIBS) \
    $(GL_LIBS) \
    $(GSTAUDIO_LIBS) \
    $(GSTVIDEO_LIBS) \
    $(GST_LIBS) \
    $(GST_RTSP_SERVER_LIBS) \
    $(GTK_LIBS) \
    $(JACK_LIBS) \
    $(LIBAVC1394_LIBS) \
    $(LIBDC1394_LIBS) \
    $(LIBRAW1394_LIBS) \
    $(top_builddir)/src/caps/libcaps.la \
    $(top_builddir)/src/devices/libdevices.la \
    $(top_builddir)/src/gst/libgst.la  \
    $(top_builddir)/src/gutil/libgutil.la \
    $(top_builddir)/src/rtsp/librtsp.la \
    $(top_builddir)/src/util/libutil.la \
    $(top_builddir)/src/vumeter/libvumeter.la

