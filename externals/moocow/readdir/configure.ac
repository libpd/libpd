dnl Process this file with autoconf to produce a configure script.
dnl -- for a clean build, run: aclocal && autoheader && automake -a && autoconf
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [readdir])
define([THE_PACKAGE_VERSION], [0.03-0])
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
## Pd external common
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: headers

dnl AC_HEADER_STDC
dnl sys/stat.h
AC_CHECK_HEADERS([dirent.h fcntl.h string.h errno.h],
	[],
	AC_MSG_WARN([-----------------------------------------------------------------])
	AC_MSG_WARN([could not find standard C headers -- things may get ugly])
	AC_MSG_WARN([-----------------------------------------------------------------]),
	[/* nonempty includes: compile only */])

## /local: headers
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: structs

##-- check for 'd_type' field in struct dirent
AC_CHECK_MEMBERS([struct dirent.d_type],
        [HAVE_STRUCT_DIRENT_D_TYPE=yes],[HAVE_STRUCT_DIRENT_D_TYPE=no],
	[#include <dirent.h>])

if test "$HAVE_STRUCT_DIRENT_D_TYPE" = "yes"; then
  ##-- check for dirent d_type decls
  AC_CHECK_DECLS([DT_REG,DT_DIR,DT_FIFO,DT_SOCK,DT_CHR,DT_BLK,DT_UNKNOWN],
	[],[],
	[#include <dirent.h>])
fi


## /local: structs
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: debug

if test "$enable_debug" = "yes"; then
  AC_DEFINE(READDIR_DEBUG,1,
	[Define this to include debugging code for the 'readdir' external.])
fi

## /local: debug
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AX_PD_LATE()
AC_OUTPUT(src/Makefile Makefile)
