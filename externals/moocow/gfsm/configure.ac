dnl Process this file with autoconf to produce a configure script.
dnl -- adapted from 'configure.in' in ggee distribution by Guenter Geiger
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [pd-gfsm])
define([THE_PACKAGE_VERSION], [0.05-2])
define([THE_PACKAGE_AUTHOR],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_AUTHOR)

##-- Additional m4 macros
AC_CONFIG_MACRO_DIR(common/m4)

##-- source & aux
AC_CONFIG_AUX_DIR(config)
dnl AC_CONFIG_AUX_DIR(.)

##-- save user's CFLAGS,CPPFLAGS
AX_PD_EARLY()

##-- use automake, autoheader
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)
AM_CONFIG_HEADER(src/config.h)

##-- Default prefix
AC_PREFIX_DEFAULT(/usr/local)

## v--- needed if Makefile.am uses _LTLIBRARIES targets
AC_PROG_LIBTOOL

dnl ----- maintainer mode
dnl  + enables "maintainer mode" only with ./configure --enable-maintainer-mode
dnl    - causes make __never__ to invoke 'config/missing', e.g. any autotools
dnl    - basically a hack to avoid version mismatches in autoconf, automake, etc.
dnl      for autobuilds from SVN
dnl  + maintainer should call ./configure --enable-maintainer-mode, and must keep
dnl    SVN sources consistent
AM_MAINTAINER_MODE
dnl -----/maintainer mode


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Pd external checks
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl ---------------------------------------------------------------
dnl pkg-config : program
dnl
AC_ARG_VAR(PKG_CONFIG, [How to run the pkg-config program])
AC_ARG_VAR(PKG_CONFIG_PATH, [Directories to search for pkg-config])
if test -z "$PKG_CONFIG" ; then
  AC_PATH_PROG(PKG_CONFIG,pkg-config,[])
fi
dnl pkg-config: destination directory
AC_ARG_WITH(pkgconfig-dir,
	AC_HELP_STRING([--with-pkgconfig-dir=DIR],
		[install pkg-config metafile(s) in DIR (default=LIBDIR/pkgconfig)]),
	[ac_cv_pkgconfigdir="$withval"])
if test -z "$ac_cv_pkgconfigdir" ; then
  ac_cv_pkgconfigdir="\$(libdir)/pkgconfig"
fi
pkgconfigdir="$ac_cv_pkgconfigdir"
AC_SUBST(pkgconfigdir)
AC_SUBST(PKG_CONFIG)
dnl
dnl pkg-config
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## gfsm:select
AC_ARG_ENABLE([system-gfsm],
	AC_HELP_STRING([--enable-system-gfsm],
		[Use system-wide (shared) libgfsm if present (default=no)]),
	[ac_cv_enable_system_gfsm="$enableval"],
	[ac_cv_enable_system_gfsm="no"])

if test "$ac_cv_enable_system_gfsm" = "yes" ; then
 PKG_CHECK_MODULES(GFSM, [gfsm >= 0.0.8], [PC_HAVE_GFSM="yes"], [PC_HAVE_GFSM=""])

 if test "$PC_HAVE_GFSM" != "yes" ; then
    AC_MSG_WARN([])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([ system-wide gfsm library not found!])
    AC_MSG_WARN([  - Is the directory containing gfsm.pc in your])
    AC_MSG_WARN([    'PKG_CONFIG_PATH' environment variable?])
    AC_MSG_WARN([  - falling back to static local libgfsm])
    AC_MSG_WARN([----------------------------------------------------------------])
    AC_MSG_WARN([])
    ac_cv_have_system_gfsm="no"
    ac_cv_enable_system_gfsm="no"
 else
    ac_cv_have_system_gfsm="yes"
 fi
fi
## /gfsm:select
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## gfsm:flags
##
AX_PD_LATE()
AC_MSG_CHECKING([which libgfsm to use])

if test "$ac_cv_enable_system_gfsm" != "yes"; then
 ##-- gfsm:local
 AC_MSG_RESULT([local])
 AC_CONFIG_SUBDIRS(gfsm)
 PD_GFSM_SUBDIRS="gfsm"

 ##-- gfsm:local: flags
 CPPFLAGS="$CPPFLAGS -I\$(top_srcdir)/gfsm/src/libgfsm -I\$(top_builddir)/gfsm/src/libgfsm \`\$(PKG_CONFIG) --cflags-only-I \$(top_builddir)/gfsm/gfsm.pc\`"
 LDFLAGS="$LDFLAGS -L\$(top_builddir)/gfsm/src/libgfsm \`\$(PKG_CONFIG) --cflags-only-I \$(top_builddir)/gfsm/gfsm.pc\`"
 LIBS="$LIBS \$(top_builddir)/gfsm/src/libgfsm/libgfsm.la \`\$(PKG_CONFIG) --libs-only-l \$(top_builddir)/gfsm/gfsm.pc | sed -e's/\-lgfsm//g'\`"

 PD_GFSM_WHICH="static local"

else
 ##-- gfsm:system
 AC_MSG_RESULT([system])
 PD_GFSM_SUBDIRS=""
 PD_GFSM_WHICH="system global"

 ##-- gfsm:system: flags
 CPPFLAGS="$CPPFLAGS `$PKG_CONFIG --cflags-only-I gfsm`"
 LDFLAGS="$LDFLAGS `$PKG_CONFIG --libs-only-L gfsm`"
 LIBS="$LIBS `$PKG_CONFIG --libs-only-l gfsm`"
fi

AC_SUBST(PD_GFSM_SUBDIRS)
AC_DEFINE_UNQUOTED(PD_GFSM_WHICH, "$PD_GFSM_WHICH", [Which version of gfsm are we using?])
dnl AC_DEFINE_UNQUOTED(PD_GFSM_DATE,  "`date`",         [Date this external was configured])
dnl AC_DEFINE_UNQUOTED(PD_GFSM_USER,  "$USER",          [User who configured this external])
## /gfsm:local
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## gfsm:any
AC_ARG_ENABLE(gfsm,
	AC_HELP_STRING([--disable-gfsm], [Globally disable gfsm support (causes build to fail)]),
	[ac_cv_enable_gfsm="$enableval"],
	[ac_cv_enable_gfsm="yes"])

if test "$ac_cv_enable_gfsm" != "yes" ; then
	AC_MSG_ERROR([gfsm support disabled by user],1)
fi
## /gfsm:any
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl AX_PD_LATE()
AC_OUTPUT(config/Makefile src/Makefile Makefile)
