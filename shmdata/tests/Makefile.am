AM_CXXFLAGS = -I$(top_srcdir)
LDADD = \
	$(top_builddir)/shmdata/libshmdata-@SHMDATA_API_VERSION@.la

check_PROGRAMS = \
    check_hello

TESTS = $(check_PROGRAMS)

check_hello_SOURCES = check_hello.cpp

