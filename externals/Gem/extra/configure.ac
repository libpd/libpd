#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.


AC_PREREQ(2.60)
AC_INIT(Gem-extras,[0.1],zmoelnig@iem.at)
AM_INIT_AUTOMAKE([1.10 foreign])

AC_CONFIG_MACRO_DIR([../m4])

AC_CONFIG_SRCDIR([README-EXTRA.txt])
AC_CONFIG_FILES([Makefile])

AC_PROG_CXX
AC_SUBST(CXX)

## pix_artoolkit
if test -d "$srcdir/pix_artoolkit"; then
  AC_CONFIG_SUBDIRS([pix_artoolkit])
fi

## pix_drum (Jaime Oliver)
#if test -d "$srcdir/pix_drum"; then
#  AC_CONFIG_SUBDIRS([pix_drum])
#fi

## pix_mano (Jaime Oliver)
#if test -d "$srcdir/pix_mano"; then
#  AC_CONFIG_SUBDIRS([pix_mano])
#fi

### pix_opencv (Yves Degoyon & Lluis Gomez i Bigorda)
#if test -d "$srcdir/pix_opencv"; then
#  AC_CONFIG_SUBDIRS([pix_opencv])
#fi

AC_OUTPUT

