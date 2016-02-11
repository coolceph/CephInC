include Makefile-env.am

bin_PROGRAMS += cceph_osd

cceph_osd_LDADD = \
	$(LIBOSD)     \
	$(LIBMESSAGE) \
	$(LIBCOMMON)

#subdirs

include common/Makefile.am
include message/Makefile.am
include osd/Makefile.am
include client/Makefile.am
include test/Makefile.am


# tests to actually run on "make check"; if you need extra, non-test,
# executables built, you need to replace this with manual assignments
# target by target

TESTS = \
	$(check_PROGRAMS)
