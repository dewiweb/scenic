SUBDIRS = tests

AM_CXXFLAGS = \
    $(BOOST_CPPFLAGS) \
    -I$(top_srcdir)

AM_LIBS = \
    $(BOOST_FILESYSTEM_LIB) \
    $(BOOST_LDFLAGS) \
    $(BOOST_LIBS)  \
    $(BOOST_SYSTEM_LIB) \
    $(BOOST_THREAD_LIB)

# use lib_LTLIBRARIES to build a shared lib:
lib_LTLIBRARIES = libshmdata-@SCENIC_API_VERSION@.la

## Instruct libtool to include ABI version information in the generated shared
## library file (.so).  The library ABI version is defined in configure.ac, so
## that all version information is kept in one place.
libshmdata_@SCENIC_API_VERSION@_la_LDFLAGS = -export-dynamic -version-info $(SCENIC_LT_VERSION) $(AM_LIBS)
libshmdata_@SCENIC_API_VERSION@_la_LIBS = $(AM_LIBS)
libshmdata_@SCENIC_API_VERSION@_la_CXXFLAGS = $(AM_CXXFLAGS)
libshmdata_@SCENIC_API_VERSION@_la_includedir = $(includedir)/shmdata-$(SCENIC_API_VERSION)/shmdata

## Define the source file list for the "libshmdata-@SCENIC_API_VERSION@.la"
## target.  Note that @SCENIC_API_VERSION@ is not interpreted by Automake and
## will therefore be treated as if it were literally part of the target name,
## and the variable name derived from that.
## PLEASE KEEP THEM IN ALPHABETICAL ORDER
libshmdata_@SCENIC_API_VERSION@_la_SOURCES = \
    shmdata.cpp

## PLEASE KEEP THEM IN ALPHABETICAL ORDER
libshmdata_@SCENIC_API_VERSION@_la_include_HEADERS = \
    shmdata.h

