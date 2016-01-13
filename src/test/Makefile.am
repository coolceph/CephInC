# target to build but not run the unit tests
unittests:: $(check_PROGRAMS)

UNITTEST_CXXFLAGS = \
        $(AM_CXXFLAGS) \
        -I$(top_builddir)/module/googletest/googletest/include \
        -I$(top_builddir)/module/bhook/include
UNITTEST_LDADD = \
        $(top_builddir)/module/googletest/googletest/lib/libgtest.la \
        $(top_builddir)/module/googletest/googletest/lib/libgtest_main.la \
        $(top_builddir)/module/bhook/libbhook.a \
	-lbfd -ldl -lopcodes -lpthread

unittest_common_log_SOURCES = test/test_common_log.cc
unittest_common_log_LDADD = $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_common_log_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_common_log

unittest_message_io_SOURCES = test/test_message_io.cc
unittest_message_io_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_io_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_io

unittest_message_messenger_SOURCES = test/test_message_messenger.cc
unittest_message_messenger_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_messenger_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_messenger

unittest_message_msg_header_SOURCES = test/test_message_msg_header.cc
unittest_message_msg_header_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_msg_header_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_msg_header

#unittest_libclient_SOURCES = test/test_libclient.cc
#unittest_libclient_LDADD = $(LIBCLIENT) $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
#unittest_libclient_CXXFLAGS = $(UNITTEST_CXXFLAGS)
#check_PROGRAMS += unittest_libclient

