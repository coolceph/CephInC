include Makefile-env.am

DIST_SUBDIRS += gtest
bin_PROGRAMS += cceph_osd
bin_PROGRAMS += tmp_client


cceph_osd_LDADD = \
	$(LIBCOMMON)

#subdirs

include client/Makefile.am
include common/Makefile.am
include test/Makefile.am


# tests to actually run on "make check"; if you need extra, non-test,
# executables built, you need to replace this with manual assignments
# target by target

TESTS = \
	$(check_PROGRAMS)
