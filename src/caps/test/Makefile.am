TESTS = caps_test
check_PROGRAMS = caps_test

caps_test_SOURCES = caps_test.cpp $(top_srcdir)/src/caps/caps_server.h $(top_srcdir)/src/caps/caps_server.cpp \
                    $(top_srcdir)/src/caps/caps_client.h $(top_srcdir)/src/caps/caps_client.cpp
caps_test_LDADD = $(top_builddir)/src/caps/libcaps.la $(top_builddir)/src/util/libutil.la $(BOOST_THREAD_LIB) $(BOOST_SYSTEM_LIB) $(GST_LIBS)
INCLUDES = -I$(top_srcdir)/src/include -I$(top_srcdir)/src
AM_CXXFLAGS = $(GST_CFLAGS)
