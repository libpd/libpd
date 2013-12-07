dnl try to figure out the target operating system and set some AM-macros accordingly 
dnl
dnl Copyright (C) 2011 IOhannes m zmölnig


AC_DEFUN([IEM_OPERATING_SYSTEM],
[
AC_CANONICAL_HOST

LINUX=no
ANDROID=no
MACOSX=no
IPHONEOS=no
BSD=no
WINDOWS=no
MINGW=no
CYGWIN=no
HURD=no
IRIX=no

case $host_os in
*linux*)
	 LINUX=yes
  ;;
*darwin*)
	 MACOSX=yes
	;;
GNU/kFreeBSD)
   BSD=yes
	;;
*mingw*)
	WINDOWS=yes
	MINGW=yes
	;;
*cygwin*)
	WINDOWS=yes
	CYGWIN=yes
  ;;
GNU)
	 HURD=yes
  ;;
esac


AM_CONDITIONAL(LINUX, test x$LINUX = xyes)
AM_CONDITIONAL(ANDROID, test x$ANDROID = xyes)
AM_CONDITIONAL(MACOSX, test x$MACOSX = xyes)
AM_CONDITIONAL(IPHONEOS, test x$IPHONEOS = xyes)
AM_CONDITIONAL(BSD, test x$BSD = xyes)
AM_CONDITIONAL(WINDOWS, test x$WINDOWS = xyes)
AM_CONDITIONAL(CYGWIN, test x$MINGW = xyes)
AM_CONDITIONAL(MINGW, test x$MINGW = xyes)
AM_CONDITIONAL(HURD, test x$HURD = xyes)
AM_CONDITIONAL(IRIX, test x$IRIX = xyes)
]) dnl IEM_OPERATING_SYSTEM
