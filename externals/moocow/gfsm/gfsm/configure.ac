dnl Process this file with autoconf to produce a configure script.
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [gfsm])
define([THE_PACKAGE_VERSION], [0.0.10-pre8-pd])
define([THE_PACKAGE_MAINTAINER],  [moocow@ling.uni-potsdam.de])

AC_INIT(THE_PACKAGE_NAME, THE_PACKAGE_VERSION, THE_PACKAGE_MAINTAINER)
dnl AC_INIT(gfsm, 0.01, moocow@ling.uni-potsdam.de)

dnl source && aux dir
AC_CONFIG_AUX_DIR(config)

dnl canonical target (sets $target, $target_(cpu|vendor|os) : used for bindist)
AC_CANONICAL_TARGET

dnl use automake
AM_INIT_AUTOMAKE(THE_PACKAGE_NAME, THE_PACKAGE_VERSION)

dnl use autoheader
AM_CONFIG_HEADER([src/libgfsm/gfsmConfigAuto.h])

dnl default prefix
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

#-------------------------------------------------------------
# save user's *FLAGS
#USER_LIBS="$LIBS"
#USER_LDFLAGS="$LDFLAGS"
#USER_CPPFLAGS="$CPPFLAGS"
USER_CFLAGS="$CFLAGS"

dnl
dnl check for programs
dnl
AC_PROG_CC
dnl AC_PROG_CXX
dnl AC_LANG(C++)

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## gnulib
gl_EARLY
gl_INIT
dnl /gnulib
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##----------------------------------------------------------------------
## DISTCHECK: simulate missing development-only packages
AC_ARG_VAR(DISTCHECK,
	[(Maintainer): set to nonempty value to simulate missing development packages])
if test -n "$DISTCHECK" ; then
   test -z "$PERL"  && PERL=no
   test -z "$OPTGEN_PERL" && OPTGEN_PERL=no
   test -z "$DOXYGEN" && DOXYGEN=no
   test -z "$POD2X" && POD2TEXT=no
   test -z "$POD2X" && POD2MAN=no
   test -z "$POD2X" && POD2HTML=no
   test -z "$POD2X" && POD2LATEX=no
   test -z "$FLEX"  && FLEX=no
   test -z "$BISON" && BISON=no
fi
## /DISTCHECK
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## flex
##
AC_ARG_VAR(FLEX, [Path to GNU flex; "no" to disable])
if test -z "$FLEX" ; then
  AC_PATH_PROG(FLEX,flex,[no])
fi

AC_MSG_NOTICE([setting FLEX=$FLEX])

if test "$FLEX" = "no"; then
  AC_MSG_WARN([GNU flex is missing, broken, or disabled])
  AC_MSG_WARN([- compilation from .l lexer source files disabled])
fi

AM_CONDITIONAL(HAVE_FLEX,[test "$FLEX" != "no"])
##
## /flex
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## bison
##
AC_ARG_VAR(BISON, [Path to GNU bison; "no" to disable])
if test -z "$BISON" ; then
  AC_PATH_PROG(BISON,bison,[no])
fi

AC_MSG_NOTICE([setting BISON=$BISON])

if test "$BISON" = "no"; then
  AC_MSG_WARN([GNU bison is missing, broken, or disabled])
  AC_MSG_WARN([- compilation from .y parser source files disabled])
fi

AM_CONDITIONAL(HAVE_BISON,[test "$BISON" != "no"])
##
## /bison
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## flex+bison: automake woes

###-- automake wants these, we want flex & bison
dnl ... and these wreak havoc
dnl AM_PROG_LEX
dnl AC_PROG_YACC
dnl
dnl ... these are goofy too
dnl LEX="$FLEX"
dnl YACC="$BISON"
dnl AC_SUBST(LEX)
dnl AC_SUBST(YACC)
dnl
dnl ... with some hacking in src/libgfsm/Makefile.am, we get:
dnl     : AM_YFLAGS = --defines --fixed-output-files --name-prefix="$*_yy"
AM_PROG_LEX
AC_PROG_YACC
LEX="$FLEX"
YACC="$BISON"
## /flex+bison: automake woes
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## zlib
##
AC_ARG_ENABLE(zlib,
	AC_HELP_STRING([--disable-zlib],
	               [Disable zlib compression support]),
	[ac_cv_enable_zlib="$enableval"],
	[ac_cv_enable_zlib="yes"])

if test "$ac_cv_enable_zlib" != "no" ; then
 for h in zlib.h ; do
  AC_CHECK_HEADER($h,
		  [gfsm_have_header="yes"], [gfsm_have_header="no"], [ ])
  if test "$gfsm_have_header" = "no" ; then
    AC_MSG_WARN([zlib header '$h' not found!])
    AC_MSG_WARN([- Is the directory containing the zlib headers in your])
    AC_MSG_WARN([  'CPPFLAGS' environment variable?])
    AC_MSG_NOTICE([zlib support disabled])
    ac_cv_enable_zlib="no"
  fi
 done
fi

if test "$ac_cv_enable_zlib" != "no" ; then
 ##-- zlib: library: compile
 AC_CHECK_LIB(z,deflate,[ac_cv_have_libz="yes"])
 if test "$ac_cv_have_libz" != "yes" ; then
    AC_MSG_WARN([could not link to zlib library!])
    AC_MSG_WARN([- Is the directory containing the zlib binary in your])
    AC_MSG_WARN([  'LDFLAGS' environment variable?])
    AC_MSG_NOTICE([zlib support disabled])
    ac_cv_enable_zlib="no"
 else
   gfsm_LIBS="$gfsm_LIBS -lz"
 fi
fi

##-- zlib: config.h flag
if test "$ac_cv_enable_zlib" != "no" ; then
 AC_DEFINE(GFSM_ZLIB_ENABLED,1,
	   [Define this to enable zlib compression support])
 DOXY_DEFINES="$DOXY_DEFINES GFSM_ZLIB_ENABLED=1"
fi
##
## /zlib
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## gnulib: funcs
##
dnl AC_CHECK_FUNCS(getline)

gl_FUNC_GETDELIM
gl_FUNC_VASPRINTF

AC_CHECK_FUNCS([vfprintf],[],[])
##
## /gnulib: funcs
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


dnl v--- needed if Makefile.am uses _LTLIBRARIES targets
AC_PROG_LIBTOOL

dnl v--- for static-only libraries (non-libtool)
dnl AC_PROG_RANLIB

dnl v--- needed if Makefile.am uses lex sources (see above)
dnl AM_PROG_LEX


### --- unmangle user *FLAGS
if test "$CFLAGS" != "$USER_CFLAGS" ; then
  # autoconf likes to set this (to '-g -O2')... who knows why...
  AC_MSG_NOTICE([Restoring user's original CFLAGS value])
  CFLAGS="$USER_CFLAGS"
fi

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## config options: init
CONFIG_OPTIONS=""
## /config options
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## inline ?
##
AC_MSG_CHECKING([whether to enable inline functions])
AC_ARG_ENABLE(inline,
	AC_HELP_STRING([--disable-inline], [disable function inlining (default=enabled)]))

if test "$enable_inline" != "no" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(GFSM_INLINE_ENABLED,1,
	[Define this to enable inline functions])
  DOXY_DEFINES="$DOXY_DEFINES GFSM_INLINE_ENABLED=1 GFSM_INLINE:=\"static inline\""
  CONFIG_INLINE_ENABLED="1"
else
  AC_MSG_RESULT(no)
  CONFIG_INLINE_ENABLED="0"
fi
CONFIG_OPTIONS="$CONFIG_OPTIONS INLINE=$CONFIG_INLINE_ENABLED"
##
## /inline ?
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## debug ?
##
AC_MSG_CHECKING([whether to build debug version])
AC_ARG_ENABLE(debug,
	AC_HELP_STRING([--enable-debug], [build debug version (default=no)]))

if test "$enable_debug" == "yes" ; then
   AC_MSG_RESULT(yes)

   dnl-- this breaks default shared-library building
   dnl-- on debian/sid:
   dnl    + automake 1.9.6
   dnl    + autoconf 2.59
   dnl    + libtool  1.5.20
   dnl
   dnl AC_DISABLE_SHARED
   if test "$GCC" == "yes" ; then
     AC_MSG_NOTICE([GNU C compiler detected: setting appropriate debug flags])
     gfsm_OFLAGS="-g"
   else
     AC_MSG_WARN([GNU C compiler not detected: you must use CFLAGS to set compiler debugging flags])
     gfsm_OFLAGS=""
   fi

   AC_DEFINE(GFSM_DEBUG_ENABLED,1,
	     [Define this to enable debugging code])
   DOXY_DEFINES="$DOXY_DEFINES GFSM_DEBUG_ENABLED=1"
   #CONFIG_OPTIONS="$CONFIG_OPTIONS DEBUG=1"
   CONFIG_OPTIONS="DEBUG=1"
else
  AC_MSG_RESULT(no)
  if test "$GCC" == "yes"; then
   case "$CFLAGS" in
    *-O*)
      AC_MSG_NOTICE([CFLAGS appears already to contain optimization flags - skipping])
      gfsm_OFLAGS=""
      ;;
    *)
     AC_MSG_NOTICE([GNU C compiler detected: setting appropriate optimization flags])
     #gfsm_OFLAGS="-O3 -finline-limit-10000 --param inline-unit-growth=1000"
     gfsm_OFLAGS="-O2"
     ;;
   esac
  else
    AC_MSG_WARN([GNU C compiler not detected: you must use CFLAGS to set compiler optimization flags])
    gfsm_OFLAGS=""
  fi
  #CONFIG_OPTIONS="$CONFIG_OPTIONS DEBUG=0"
  CONFIG_OPTIONS="DEBUG=0"
fi

AC_SUBST(gfsm_OFLAGS)
##
## /debug ?
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

#---------------------------------------------------------------
# short package includes
spkgincludedir="\${includedir}/AC_PACKAGE_NAME"
AC_SUBST(spkgincludedir)
# short package includes
#---------------------------------------------------------------

#---------------------------------------------------------------
# get real prefix
AC_CACHE_CHECK([for installation prefix], [ac_cv_install_prefix],
	[if test "$prefix" = "NONE" ; then
	   ac_cv_install_prefix="/usr/local"
	 else
           ac_cv_install_prefix="$prefix"
	 fi
	])
dnl #  ... and add it to our flags
dnl CPPFLAGS="$CPPFLAGS -I${ac_cv_install_prefix}/include"
dnl LDFLAGS="$LDFLAGS -L${ac_cv_install_prefix}/lib"


dnl check for strdup
AC_CHECK_FUNC(strdup,[AC_DEFINE(HAVE_STRDUP,1,[Define this if you have the strdup() function])])


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
dnl
dnl pkg-config
dnl ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## glib (pkg-config)
PKG_CHECK_MODULES(GLIB, glib-2.0 >= 2.0.0,
	[PC_HAVE_GLIB="yes"],
	[PC_HAVE_GLIB=""])

if test "$PC_HAVE_GLIB" != "yes" ; then
    AC_MSG_WARN([glib-2.0 not found via pkg-config])
    AC_MSG_WARN([- Is the directory containing glib-2.0.pc in your])
    AC_MSG_WARN([  'PKG_CONFIG_PATH' environment variable?])
    AC_MSG_WARN([- I'll assume that you have set the variables CFLAGS, LDFLAGS,])
    AC_MSG_WARN([   and LIBS for compling and linking with glib-2.0])
    ac_cv_enable_glib="no"
else
    ac_cv_enable_glib="yes"
fi

if test "$ac_cv_enable_glib" != "no" ; then
  ##-- glib: library
  ## + we use "gfsm_CFLAGS" here because CFLAGS gets clobbered if
  ##   we're called from a higher-level Makefile (e.g. pd-extended)
  gfsm_CFLAGS="$gfsm_CFLAGS `$PKG_CONFIG --cflags glib-2.0`"
  gfsm_LDFLAGS="$gfsm_LDFLAGS `$PKG_CONFIG --libs-only-L glib-2.0`"
  gfsm_LIBS="$gfsm_LIBS `$PKG_CONFIG --libs-only-l glib-2.0`"
fi
##
## /glib
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## version-info
GFSM_VERSION_MAJOR=`[echo ${PACKAGE_VERSION} | sed -e's/^\([0-9][0-9]*\)\..*/\1/']`
test -z "$GFSM_VERSION_MAJOR" -o "$GFSM_VERSION_MAJOR" = "$PACKAGE_VERSION" \
  && GFSM_VERSION_MAJOR=0

GFSM_VERSION_MINOR=`[echo ${PACKAGE_VERSION} | sed -e's/^[^\.]*\.0*\([0-9][0-9]*\).*/\1/']`
test -z "$GFSM_VERSION_MINOR" -o "$GFSM_VERSION_MINOR" = "$PACKAGE_VERSION" \
  && GFSM_VERSION_MINOR=0

GFSM_VERSION_MICRO=`[echo ${PACKAGE_VERSION} | sed -e's/^[^\.]*\.[^\.]*\.0*\([0-9][0-9]*\).*/\1/']`
test -z "$GFSM_VERSION_MICRO" -o "$GFSM_VERSION_MICRO" = "$PACKAGE_VERSION" \
  && GFSM_VERSION_MICRO=0

AC_DEFINE_UNQUOTED(GFSM_VERSION_MAJOR, $GFSM_VERSION_MAJOR, [Major gfsm version])
AC_DEFINE_UNQUOTED(GFSM_VERSION_MINOR, $GFSM_VERSION_MINOR, [Minor gfsm version])
AC_DEFINE_UNQUOTED(GFSM_VERSION_MICRO, $GFSM_VERSION_MICRO, [Micro gfsm version])
AC_SUBST(GFSM_VERSION_MAJOR)
AC_SUBST(GFSM_VERSION_MINOR)
AC_SUBST(GFSM_VERSION_MICRO)
## /version-info
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## exp()
AC_CHECK_FUNC(exp,[gfsm_have_exp="yes"],[gfsm_have_exp=""])
if test "$gfsm_have_exp" != "yes" ; then
  AC_CHECK_LIB(m,exp,[gfsm_have_exp="yes"],[gfsm_have_exp=""])
  if test "$gfsm_have_exp" != "yes" ; then
    AC_MSG_ERROR([C function exp() not found: oops!])
  else
    gfsm_LIBS="$gfsm_LIBS -lm"
  fi
fi
## /exp()
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## perl
## --> REMOVED for pd-local libgfsm
PERL=""
AC_SUBST(PERL)
## /perl
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## optgen.perl & Getopt::Gen
## --> REMOVED for pd-local libgfsm
##
OPTGEN_PERL="no"
AC_SUBST(OPTGEN_PERL)
## /optgen.perl
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Documentation
## --> REMOVED for pd-local libgfsm
  AC_ARG_VAR(POD2TEXT, [Path to perl 'pod2text' script; "no" for none])
  AC_ARG_VAR(POD2HTML, [Path to perl 'pod2html' script; "no" for none])

  if test -z "$POD2TEXT" ; then
    AC_PATH_PROG(POD2TEXT,pod2text,[no])
  fi
  if test -z "$POD2HTML" ; then
    AC_PATH_PROG(POD2HTML,pod2html,[no])
  fi

  AC_SUBST(POD2TEXT)
  AC_SUBST(POD2HTML)

AM_CONDITIONAL(HAVE_POD2TEXT,    [test -n "$POD2TEXT"  -a "$POD2TEXT"  != "no"])
AM_CONDITIONAL(HAVE_POD2HTML,    [test -n "$POD2HTML"  -a "$POD2HTML"  != "no"])

## /Documentation
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## subdirs
## --> REMOVED for pd-local libgfsm
##
GFSM_SUBDIRS=""
GFSM_SRC_SUBDIRS=""
AC_SUBST(GFSM_SUBDIRS)
AC_SUBST(GFSM_SRC_SUBDIRS)
## /subdirs
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


##---------------------------------------------------------------
## Binary distribution
## --> REMOVED for pd-local libgfsm
##
## Binary distribution
##---------------------------------------------------------------


##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## warnings ?
##
AC_MSG_CHECKING([whether to display compiler warnings])
AC_ARG_ENABLE(warnings,
	AC_HELP_STRING([--disable-warnings],[disable compiler warnings (default=no)]))

if test "$enable_warnings" != "no" ; then
   AC_MSG_RESULT(yes)
   if test "$GCC" == "yes"; then
     AC_MSG_NOTICE([GNU C compiler detected: setting appropriate warning flags])
     gfsm_WFLAGS="-Wall" #-Winline
   else
     AC_MSG_NOTICE([GNU C compiler not detected: not setting warning flags])
     gfsm_WFLAGS=""
   fi
else
  AC_MSG_RESULT(no)
  gfsm_WFLAGS=""
fi
AC_SUBST(gfsm_WFLAGS)
##
## /warnings
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## autotest
## --> REMOVED for pd-local libgfsm
dnl AC_CONFIG_TESTDIR(tests,tests)
## /autotest
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## no-autoheader
AC_CONFIG_LINKS(src/libgfsm/gfsmConfigNoAuto.h:src/libgfsm/gfsmConfigAuto.h.in)
## /no-autoheader
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## configuration options
AC_SUBST(CONFIG_OPTIONS)
AC_SUBST(gfsm_CFLAGS)
AC_SUBST(gfsm_LDFLAGS)
AC_SUBST(gfsm_LIBS)
## /config options
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

dnl
dnl output
dnl
AC_CONFIG_FILES(src/libgfsm/gnulib/Makefile)
AC_CONFIG_FILES(src/libgfsm/Makefile)
dnl AC_CONFIG_FILES(src/programs/Makefile)
dnl AC_CONFIG_FILES(src/programs/gfsmconfig, [chmod 0755 src/programs/gfsmconfig])
AC_CONFIG_FILES(src/Makefile)
dnl AC_CONFIG_FILES(doc/libgfsm/mainpage.dox doc/libgfsm/libgfsm.doxy doc/libgfsm/Makefile)
dnl AC_CONFIG_FILES(doc/programs/Makefile)
dnl AC_CONFIG_FILES(doc/tutorial/Makefile)
dnl AC_CONFIG_FILES(doc/Makefile)
AC_CONFIG_FILES(config/Makefile)
dnl AC_CONFIG_FILES(tests/atlocal tests/Makefile)
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(gfsm.pc)
AC_OUTPUT
