noinst_LTLIBRARIES = libgst.la

libgst_la_SOURCES = \
    audio_config.cpp \
    audio_factory.cpp \
    audio_level.cpp \
    audio_receiver.cpp \
    audio_sender.cpp \
    audio_sink.cpp \
    audio_source.cpp \
    bus_msg_handler.cpp \
    codec.cpp \
    dv1394.cpp \
    file_source.cpp \
    gst_linkable.cpp \
    jack_util.cpp \
    local_audio.cpp \
    local_video.cpp \
    media_base.cpp \
    pipeline.cpp \
    remote_config.cpp \
    rtp_bin.cpp \
    rtp_pay.cpp \
    rtp_receiver.cpp \
    rtp_sender.cpp \
    shared_video_sink.cpp \
    text_overlay.cpp \
    video_config.cpp \
    video_factory.cpp \
    video_flip.cpp \
    video_receiver.cpp \
    video_scale.cpp \
    video_sender.cpp \
    video_sink.cpp \
    video_source.cpp

EXTRA_DIST = \
    $(top_srcdir)/src/include/noncopyable.h \
    audio_config.h \
    audio_factory.h \
    audio_level.h \
    audio_receiver.h \
    audio_sender.h \
    audio_sink.h \
    audio_source.h \
    bus_msg_handler.h \
    codec.h \
    dv1394.h \
    file_source.h \
    gst_linkable.h \
    jack_util.h \
    local_audio.h \
    local_video.h \
    media_base.h \
    pipeline.h \
    port_offsets.h \
    remote_config.h \
    rtp_bin.h \
    rtp_pay.h \
    rtp_receiver.h \
    rtp_sender.h \
    shared_video_sink.h \
    text_overlay.h \
    video_config.h \
    video_factory.h \
    video_flip.h \
    video_receiver.h \
    video_scale.h \
    video_sender.h \
    video_sink.h \
    video_source.h

libgst_la_CPPFLAGS = \
    $(BOOST_CPPFLAGS) \
    $(GLIB_CFLAGS) \
    $(GLU_CFLAGS) \
    $(GSTAPP_CFLAGS) \
    $(GST_CFLAGS) \
    $(GTK_CFLAGS) \
    $(JACK_CFLAGS) \
    $(LIBAVC1394_CFLAGS) \
    -DPIXMAPS_DIR=\"$(PIXMAPS_DIR)\" \
    -I$(top_srcdir)/src \
    -I$(top_srcdir)/src/include

libgst_la_LIBADD = \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_LDFLAGS) \
    $(BOOST_SYSTEM_LIB) \
    $(GSTAPP_LIBS) \
    $(GSTAUDIO_LIBS) \
    $(GSTVIDEO_LIBS) \
    $(GST_LIBS) \
    $(GTK_LIBS) \
    $(JACK_LIBS) \
    $(LIBAVC1394_LDFLAGS) \
    $(LIBDC1394_LIBS) \
    $(LIBRAW1394_LIBS) \
    $(top_builddir)/src/caps/libcaps.la \
    $(top_builddir)/src/devices/libdevices.la

