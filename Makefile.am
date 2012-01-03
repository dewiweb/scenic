SUBDIRS = src po data py doc utils man
ACLOCAL_AMFLAGS = -I m4

# "foreign" means not GNU, and is common for avoiding boring messages
# about files organized differently from what GNU expects
AUTOMAKE_OPTIONS = foreign

EXTRA_DIST = \
    RELEASE \
    NEWS \
    README \
    autogen.sh \
    configure.ac \
    scenic.pc.in \
    scenic.pc

DISTCLEANFILES = shared-video.pc
pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = shared-video-@SHAREDVIDEO_API_VERSION@.pc

