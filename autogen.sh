#!/bin/sh -x
rm -f config.cache
libtoolize --force --copy
aclocal
autoconf
automake -a --add-missing -Wall
( cd src/gtest && autoreconf -fvi; )
