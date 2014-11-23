#!/bin/sh -x
rm -f config.cache
libtoolize
aclocal
autoconf
automake -a --add-missing -Wall
