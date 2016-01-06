#!/bin/sh -x

touch AUTHORS
touch ChangeLog
touch INSTALL
touch NEWS
touch README

rm -f config.cache
libtoolize --force --copy
aclocal
autoconf
automake -a --add-missing -Wall
( cd module/googletest/googletest && autoreconf -fvi; )
( cd module/bhook && ./autogen.sh; )

./configure
