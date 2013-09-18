dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

##-- Some handy macros
define([THE_PACKAGE_NAME],    [pdstring])
define([THE_PACKAGE_VERSION], [0.10-2])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(common/m4)

##-- source & aux
AC_CONFIG_AUX_DIR(config)

##-- save user's CFLAGS,CPPFLAGS
AX_PD_EARLY()

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

##-- Default prefix
AC_PREFIX_DEFAULT(/usr/local)

dnl ----- maintainer mode
dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
dnl    - causes make __never__ to invoke 'config/missing' (i.e. any autotools)
dnl    - basically a hack to avoid version mismatches in autoconf, automake, etc.
dnl      for autobuilds from SVN
dnl  + maintainer should call ./configure --enable-maintainer-mode, and must keep
dnl    SVN sources consistent
AM_MAINTAINER_MODE
dnl -----/maintainer mode

##-- other flags (?)
dnl AC_ISC_POSIX

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Pd external checks
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: headers
dnl AC_HEADER_STDC
AC_CHECK_HEADERS([string.h],
	[],
	AC_MSG_WARN([-----------------------------------------------------------------])
	AC_MSG_WARN([could not find standard C headers -- things may get ugly])
	AC_MSG_WARN([-----------------------------------------------------------------]),
	[/* nonempty includes: compile only */])
## /local: headers
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##-- local: debugging
if test "$enable_debug" = "yes" ; then
  AC_DEFINE(ANY2BYTES_DEBUG,1,
	[Define this to include debugging code for the 'bytes2any' external.])
  AC_DEFINE(BYTES2ANY_DEBUG,1,
	[Define this to include debugging code for the 'any2bytes' external.])
fi


##-- local: output
AX_PD_LATE()
AC_CONFIG_LINKS([src/any2string-help.pd:src/any2bytes-help.pd])
AC_CONFIG_LINKS([src/string2any-help.pd:src/bytes2any-help.pd])
AC_OUTPUT(config/Makefile src/Makefile Makefile)
