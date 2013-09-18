#!/bin/sh

#-----------------------------------------------------------------------
# File: autogen.sh
# Description:
#   + wrapper for m4 black-magic
#-----------------------------------------------------------------------

MY_ALDIRS="."
MY_AHDIRS=""
MY_AMDIRS="."
MY_ACDIRS="."

test -z "$ACLOCAL" && ACLOCAL=aclocal
test -z "$AUTOHEADER" && AUTOHEADER=autoheader
test -z "$AUTOMAKE" && AUTOMAKE=automake
test -z "$AUTOCONF" && AUTOCONF=autoconf

if test -n "$MY_ALDIRS"; then
 for d in $MY_ALDIRS ; do
    echo "(cd $d ; $ACLOCAL)"
    (cd $d ; $ACLOCAL)
 done
fi

if test -n "$MY_AHDIRS"; then
 for d in $MY_AHDIRS ; do
    echo "(cd $d ; $AUTOHEADER)"
    (cd $d ; $AUTOHEADER)
 done
fi

if test -n "$MY_AMDIRS"; then
 for d in $MY_AMDIRS ; do
    echo "(cd $d ; $AUTOMAKE -a)"
    (cd $d ; $AUTOMAKE -a)
 done
fi

if test -n "$MY_ACDIRS"; then
 for d in $MY_ACDIRS ; do
    echo "(cd $d ; $AUTOCONF)"
    (cd $d ; $AUTOCONF)
 done
fi

#echo "(./configure)"
#./configure $*
