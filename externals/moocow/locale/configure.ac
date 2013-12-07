dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [locale])
define([THE_PACKAGE_VERSION], [0.02-1])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(m4)

##-- source & aux
AC_CONFIG_AUX_DIR(config)
dnl AC_CONFIG_AUX_DIR(.)

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

##-- Default prefix
AC_PREFIX_DEFAULT(/usr/local)

dnl ----- maintainer mode
dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
dnl    - causes make __never__ to invoke 'config/missing', e.g. any autotools
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
AX_PD_EARLY
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: headers

dnl AC_HEADER_STDC
AC_CHECK_HEADERS([string.h wchar.h locale.h],
	[],
	AC_MSG_WARN([-----------------------------------------------------------------])
	AC_MSG_WARN([could not find standard C headers -- things may get ugly])
	AC_MSG_WARN([-----------------------------------------------------------------]),
	[/* nonempty includes: compile only */])

##-- check for LC_* declarations required by ISO-C99 (ISO/IEC 9899:TC2)
AC_CHECK_DECLS([LC_ALL],
		[],
		[AC_MSG_ERROR([LC_ALL not declared - bailing out!])],
		[#include <locale.h>])
AC_CHECK_DECLS([LC_COLLATE,LC_CTYPE,LC_MONETARY,LC_NUMERIC,LC_TIME],[],[],[#include <locale.h>])

##-- check for additional LC_* declarations
AC_CHECK_DECLS([LC_MESSAGES,LC_PAPER,LC_NAME,LC_ADDRESS,LC_TELEPHONE,LC_MEASUREMENT,LC_IDENTIFICATION],[],[],[#include <locale.h>])

## /local: headers
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: debug
if test "$enable_debug" = "yes"; then
  AC_DEFINE(LOCALE_DEBUG,1,
	[Define this to include debugging code for the 'locale' external.])
fi
## /local: debug
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##-- restore user's CFLAGS
#CFLAGS="$UCFLAGS"
#CPPFLAGS="$UCPPFLAGS"

AX_PD_LATE() ##-- pd external: late
AC_OUTPUT(config/Makefile src/Makefile Makefile)
