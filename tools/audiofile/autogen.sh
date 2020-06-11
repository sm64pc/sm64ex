#!/bin/sh
# Run this to generate all the initial makefiles, etc.

test -n "$srcdir" || srcdir=`dirname $0`
test -n "$srcdir" || srcdir=.

AUTORECONF=`which autoreconf`
if test -z $AUTORECONF; then
	echo "autoreconf was not found; you must install automake, autoconf,"
	echo "and libtool to build libaudiofile."
	exit 1
fi

rootme=`pwd`
cd $srcdir
if [ ! -d "m4" ]; then
	echo "m4 doesn't exist. Creating it."
	mkdir m4
else
	echo "m4 already exists. Going straight to auto-generation."
fi
autoreconf --install --verbose || exit $?
cd $rootme

$srcdir/configure "$@"

echo 
echo "Now type 'make' to compile libaudiofile."
