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


unittest_librados_SOURCES = test/test_librados.cc
unittest_librados_LDADD = $(LIBRADOS) $(LIBCLIENT) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_librados_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_librados
