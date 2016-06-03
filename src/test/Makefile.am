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

unittest_common_buffer_SOURCES = test/test_common_buffer.cc
unittest_common_buffer_LDADD = $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_common_buffer_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_common_buffer

unittest_common_log_SOURCES = test/test_common_log.cc
unittest_common_log_LDADD = $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_common_log_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_common_log

unittest_common_option_SOURCES = test/test_common_option.cc
unittest_common_option_LDADD = $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_common_option_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_common_option

unittest_message_io_SOURCES = test/test_message_io.cc
unittest_message_io_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_io_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_io

unittest_message_messenger_SOURCES = test/test_message_messenger.cc
unittest_message_messenger_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_messenger_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_messenger

unittest_message_server_messenger_SOURCES = test/test_message_server_messenger.cc
unittest_message_server_messenger_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_server_messenger_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_server_messenger

unittest_message_msg_header_SOURCES = test/test_message_msg_header.cc
unittest_message_msg_header_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_msg_header_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_msg_header

unittest_message_msg_write_obj_SOURCES = test/test_message_msg_write_obj.cc
unittest_message_msg_write_obj_LDADD = $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_message_msg_write_obj_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_message_msg_write_obj

unittest_os_types_SOURCES = test/test_os_types.cc
unittest_os_types_LDADD = $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_os_types_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_os_types

unittest_os_transaction_SOURCES = test/test_os_transaction.cc
unittest_os_transaction_LDADD = $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_os_transaction_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_os_transaction

unittest_os_mem_store_SOURCES = test/test_os_mem_store.cc
unittest_os_mem_store_LDADD = $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_os_mem_store_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_os_mem_store

unittest_os_mem_store_coll_node_SOURCES = test/test_os_mem_store_coll_node.cc
unittest_os_mem_store_coll_node_LDADD = $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_os_mem_store_coll_node_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_os_mem_store_coll_node

unittest_os_mem_store_object_node_SOURCES = test/test_os_mem_store_object_node.cc
unittest_os_mem_store_object_node_LDADD = $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_os_mem_store_object_node_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_os_mem_store_object_node

unittest_osd_osd_SOURCES = test/test_osd_osd.cc
unittest_osd_osd_LDADD = $(LIBOSD) $(LIBMESSAGE) $(LIBOS) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_osd_osd_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_osd_osd

unittest_client_client_SOURCES = test/test_client_client.cc
unittest_client_client_LDADD = $(LIBCLIENT) $(LIBMESSAGE) $(LIBCOMMON) -lm $(UNITTEST_LDADD)
unittest_client_client_CXXFLAGS = $(UNITTEST_CXXFLAGS)
check_PROGRAMS += unittest_client_client

