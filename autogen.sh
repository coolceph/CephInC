#!/bin/sh -x

if [ `which libtoolize` ]; then
    LIBTOOLIZE=libtoolize
elif [ `which glibtoolize` ]; then
    LIBTOOLIZE=glibtoolize
else
  echo "Error: could not find libtoolize"
  echo "  Please install libtoolize or glibtoolize."
  exit 1
fi

rm -f config.cache
#aclocal -I m4 --install
#$LIBTOOLIZE --force --copy
#aclocal -I m4 --install
autoconf
#autoheader
automake -a --add-missing -Wall
