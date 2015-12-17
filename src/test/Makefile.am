# target to build but not run the unit tests
unittests:: $(check_PROGRAMS)

UNITTEST_CXXFLAGS = \
        $(AM_CXXFLAGS) \
        -I$(top_builddir)/module/gtest/include \
        -I$(top_builddir)/module/gtest/include
UNITTEST_LDADD = \
        $(top_builddir)/module/gtest/lib/libgtest.a \
        $(top_builddir)/module/gtest/lib/libgtest_main.a \
        -lpthread


unittest_libclient_SOURCES = test/test_libclient.cc
unittest_libclient_LDADD = $(LIBCLIENT) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_libclient_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_libclient
