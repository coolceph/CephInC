AUTOMAKE_OPTIONS = gnu
SUBDIRS = . src

fullcheck:
	export CCEPH_FULL_CHECK=1 && make check

check-local: all
	@cd module/googletest/googletest && $(MAKE) $(AM_MAKEFLAGS) lib/libgtest.la lib/libgtest_main.la
	@cd module/bhook && $(MAKE) $(AM_MAKEFLAGS)

clean-local:
	@if test -e module/gtest/Makefile; then \
	  echo "Making clean in src/gtest"; \
	  cd module/gtest && $(MAKE) $(AM_MAKEFLAGS) clean; \
	fi
