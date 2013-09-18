dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

##-- Some handy macros
define([THE_PACKAGE_NAME],    [hello])
define([THE_PACKAGE_VERSION], [0.01-1])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(common/m4)

##-- source & aux
AC_CONFIG_AUX_DIR(.)
dnl AC_CONFIG_AUX_DIR(config)

##-- save user's CFLAGS,CPPFLAGS
AX_PD_EARLY()

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)
dnl AM_CONFIG_HEADER(src/config.h)

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
## Pd external common
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: headers
AC_MSG_NOTICE([performing local configuration actions])
## /local: headers
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: debug
if test "$enable_debug" = "yes"; then
  AC_DEFINE(LOCAL_DEBUG,1,[Define this to enable local debugging])
fi
## /local: debug
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##-- local: output
AX_PD_LATE()
AC_OUTPUT(Makefile)
