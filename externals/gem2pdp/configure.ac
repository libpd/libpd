dnl Process this file with autoconf to produce a configure script.

AC_INIT
GEM2PDP_VERSION=0.7

if test -d /usr/local/pd; 
then
	PD_DIR=/usr/local/pd
elif test -d ../../pd; 
then
	PD_DIR=../../pd
else
   echo "Pd sources not found"
fi

if test -d /usr/local/pd/pdp; 
then
	PDP_DIR=/usr/local/pd/pdp
elif test -d ../pdp; 
then
	PDP_DIR=../pdp
else
   echo "PdP sources not found"
fi

if test -d /win/Georg/pd-cvs/gem/Gem; 
then
   GEM_DIR=/win/Georg/pd-cvs/gem/Gem
elif test -d ../../gem/Gem; 
then
	GEM_DIR=../../gem/Gem
elif  test -d ../../gem/Gem; 
then
	GEM_DIR=../../gem/Gem
else
   echo "Gem sources not found"
fi

AC_ISC_POSIX
AC_PROG_CC
AM_PROG_CC_STDC
AC_HEADER_STDC
AC_PROG_INSTALL
AC_PROG_MAKE_SET
AM_SANITY_CHECK

AM_MAINTAINER_MODE
AM_WITH_DMALLOC

AC_SUBST(GEM2PDP_VERSION)

AC_ARG_WITH(pddir,       [  --with-pddir=<dir>      specify an alternate pd source tree])
AC_ARG_WITH(gemdir,      [  --with-gemdir=<dir>     specify an alternate gem source tree])
AC_ARG_WITH(pdpdir,      [  --with-pdpdir=<dir>     specify an alternate pdp source tree])

if test "$with_gemdir" != ""
then
 GEM_DIR=$with_gemdir
fi

if test "$with_pdpdir" != ""
then
 PDP_DIR=$with_pdpdir
fi

if test "$with_pddir" != ""
then
 PD_DIR=$with_pddir
fi

echo -n "looking for gem sources (required) ... "
if test -f $GEM_DIR/src/Base/CPPExtern.h
then
  AC_SUBST(GEM_DIR)
  echo "ok." 
else
  echo "gem source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi

AC_SUBST(PD_DIR)

echo -n "looking for pdp sources (required) ... "
if test -f $PDP_DIR/include/pdp.h
then
  AC_SUBST(PDP_DIR)
  echo "ok." 
else
  echo "pdp source tree not found... install it, fix the path in configure.ac and run autoconf"
  exit -1
fi


AC_CANONICAL_HOST
case "$host" in
 	*-linux*)
		  EXTENSION=pd_linux
		  LDFLAGS="$LDFLAGS -Wl,--export-dynamic -shared"
		  ;;
 	*-mingw*)
		  CFLAGS="$CFLAGS -mms-bitfields"
		  EXTENSION=dll
		  LDFLAGS="$LDFLAGS -shared"
		  ;;
   *-darwin* | *-macos10*)
	  EXTENSION=pd_darwin
	  LDFLAGS="$LDFLAGS -bundle -undefined dynamic_lookup"
  	  # Check for DarwinPorts and/or Fink on Mac OS X/Darwin
     if test -d /sw ; then
       # Fink
       PATH="/sw/bin:/sw/sbin:$PATH"
       INCLUDES="$INCLUDES -I/sw/include"
       LIBS="$LIBS -L/sw/lib"
     elif test -d /opt/local ; then
       # DarwinPorts
       PATH="/opt/local/bin:/opt/local/sbin:$PATH"
       INCLUDES="$INCLUDES -I/opt/local/include"
       LIBS="$LIBS -L/opt/local/lib"
     fi
  ;;
esac

AC_SUBST(CFLAGS)
AC_SUBST(EXTENSION)
AC_SUBST(INCLUDES)
AC_SUBST(LDFLAGS)

LIBS="$LIBS -lm"

AC_CONFIG_FILES([
Makefile
])
AC_OUTPUT
