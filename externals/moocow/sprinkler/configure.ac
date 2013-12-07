dnl Process this file with autoconf to produce a configure script.
dnl -- adapted from 'configure.in' in ggee distribution by Guenter Geiger
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [sprinkler])
define([THE_PACKAGE_VERSION], [0.06-2])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(common/m4)

##-- source & aux
dnl AC_CONFIG_AUX_DIR(config)
AC_CONFIG_AUX_DIR(.)

##-- save user's CFLAGS,CPPFLAGS
AX_PD_EARLY()

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(config.h)

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
## Local: misc

##-- build backwards-compatible non-max 'forward'?
AC_MSG_CHECKING([whether to allow non-MAX "forward" alias])
AC_ARG_ENABLE([forward],
	AC_HELP_STRING(
	  [--enable-forward],
	  [allow backwards-compatible non-MAX "forward" alias (default=no)]))

if test "$enable_forward" = "yes" ; then
  AC_MSG_RESULT(yes)
  NON_MAX_FORWARD="yes"
  AC_DEFINE(NON_MAX_FORWARD,1,[Define this to enable MAX-incompatible "forward" alias])
  SPRINKLER_COMPAT_TEXT=["(with MAX-incompatible [forward] alias)"]
  SPRINKLER_ALIAS_1="#X obj 102 407 forward;"
  SPRINKLER_ALIAS_2="#X obj 102 432 fw;"
else
  AC_MSG_RESULT(no)
  NON_MAX_FORWARD="no"
  SPRINKLER_COMPAT_TEXT=["(formerly MAX-incompatible \[forward\])"]
  SPRINKLER_ALIAS_1="#X text 102 407 none;"
  SPRINKLER_ALIAS_2="#X text 102 432 (disabled);"
fi
AC_SUBST(NON_MAX_FORWARD)
AC_SUBST(SPRINKLER_COMPAT_TEXT)
AC_SUBST(SPRINKLER_ALIAS_1)
AC_SUBST(SPRINKLER_ALIAS_2)

##-- use pd_forwardmess() for everything?
AC_MSG_CHECKING([whether to use pd_forwardmess() for all messages])
AC_ARG_ENABLE([all-forwardmess],
	AC_HELP_STRING(
	  [--enable-all-forwardmess],
	  [use pd_forwardmess() for all messages (default=no)]))

if test "$enable_all_forwardmess" = "yes" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(ALL_FORWARDMESS, 1, [Define this to use pd_forwardmess() for all messages])
else
  AC_MSG_RESULT(no)
fi

## /local: misc
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: debug

if test "$enable_debug" = "yes" ; then
  AC_DEFINE(SPRINKLER_DEBUG,1,
	[Define this to include debugging code for the 'readdir' external.])
fi

## /local: debug
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AX_PD_LATE()
AC_OUTPUT(sprinkler-help.pd Makefile)
