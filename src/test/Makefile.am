# target to build but not run the unit tests
unittests:: $(check_PROGRAMS)

UNITTEST_CXXFLAGS = \
        $(AM_CXXFLAGS) \
        -I$(top_srcdir)/src/gtest/include \
        -I$(top_builddir)/src/gtest/include
UNITTEST_LDADD = \
        $(top_builddir)/src/gtest/lib/libgtest.a \
        $(top_builddir)/src/gtest/lib/libgtest_main.a \
        -lpthread


test_libclient_SOURCES = test/test_libclient.cc
test_libclient_LDADD = $(LIBCLIENT) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
test_libclient_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += test_libclient
