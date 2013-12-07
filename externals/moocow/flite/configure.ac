dnl Process this file with autoconf to produce a configure script.
dnl -- adapted from 'configure.in' in ggee distribution by Guenter Geiger
AC_PREREQ(2.5)

dnl Some handy macros
define([THE_PACKAGE_NAME],    [pdflite])
define([THE_PACKAGE_VERSION], [0.02-3])
define([THE_PACKAGE_AUTHOR],  [moocow.bovine@gmail.com])

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
AM_CONFIG_HEADER(config.h)

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

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Pd external checks
AX_PD_EXTERNAL
## /pd external common
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: flite

##-- flite source directory
AC_ARG_WITH(flite-dir,
	AC_HELP_STRING([--with-flite-dir=DIR], [Flite source directory (default=none)]),
	[FLITE_DIR="$withval"],
	)
if test -n "$FLITE_DIR"; then
  CPPFLAGS="$CPPFLAGS -I${FLITE_DIR}/include"
  LDFLAGS="$LDFLAGS -L${FLITE_DIR}/lib"
fi

##-- check: headers: flite.h
AC_CHECK_HEADER([flite/flite.h],[pdflite_have_header=yes],[pdflite_have_header=no],[ ])
if test "$pdflite_have_header" != "yes"; then
  AC_MSG_WARN([Required header 'flite.h' not found])
  AC_MSG_WARN([- is the directory containing 'flite.h' in your CPPFLAGS variable?])
  AC_MSG_ERROR([Required header not found: aborting],1)
fi

##-- check libs: libflite.a
LDFLAGS_SAVE="$LDFLAGS"
AC_CHECK_LIB(flite,[flite_init],[pdflite_have_lib=yes],[pdflite_have_lib=no],[-lm])
if test "$pdflite_have_lib" != "yes"; then
  for d in \
    /usr/lib /usr/local/lib /sw/lib \
    /usr/lib/flite /usr/local/lib/flite /sw/lib/flite \
    /usr/flite/lib /usr/local/flite/lib /sw/flite/lib \
    /usr/lib/flite/lib /usr/local/lib/flite/lib /sw/lib/flite/lib
  do
    LDFLAGS="$LDFLAGS_SAVE -L$d"
    AC_CHECK_LIB(flite,[flite_init],[pdflite_have_lib=yes],[pdflite_have_lib=no],[-lm])
    if test "$pdflite_have_lib" = "yes"; then
      break
    fi
  done
  ##
  ##-- libflite.a: final check
  if test "$pdflite_have_lib" != "yes"; then
    AC_MSG_WARN([Required library 'flite' not found])
    AC_MSG_WARN([- is the directory containing the 'flite' library in your LDFLAGS variable?])
    AC_MSG_ERROR([Required library not found: aborting],1)
  fi
fi

##-- flite: libs
LIBS="$LIBS -lflite -lm"
AC_SUBST(LIBS)

##-- Check for flite : libs : flite_cmulex
AC_CHECK_LIB(flite_cmulex,[cmu_lex_init],[],
	[AC_MSG_ERROR([required library 'flite_cmulex' not found -- quitting],1)],
	[])
LIBS="$LIBS -lflite_cmulex"

##-- Check for flite : libs : flite_usenglish
AC_CHECK_LIB(flite_usenglish,[usenglish_init],[],
	[AC_MSG_ERROR([required library 'flite_usenglish' not found -- quitting],1)],
	[-lflite_cmulex -lflite -lm])
LIBS="$LIBS -lflite_usenglish"

##-- Check for flite : libs : kal16, kal, awb, rms, slt
VOICELIBS=""
define([check_flite_voice],
	[
	 if test -z "$VOICELIBS" ; then
	  AC_CHECK_LIB([flite_cmu_us_[$1]],[register_cmu_us_[$1]],
		[
		 AC_DEFINE([PDFLITE_REGISTER_VOICE],
			[register_cmu_us_[$1]],
			[This is the function we will call to register the synth voice])
		 VOICELIBS="$[]VOICELIBS -lflite_cmu_us_[$1]"
		],
		[AC_MSG_WARN([could not find flite voice library '[$1]'])]
		[])
	 fi
	])

check_flite_voice(kal16)
check_flite_voice(kal)
check_flite_voice(awb)
check_flite_voice(rms)
check_flite_voice(slt)

if test -z "$VOICELIBS" ; then
  AC_MSG_ERROR([could not find any flite voice libraries!],1)
fi
LIBS="$LIBS $VOICELIBS"

## /local: flite
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
## Local: debug

if test "$enable_debug" = "yes" ; then
  DEBUG="yes"
  DFLAGS="$DFLAGS -DPDFLITE_DEBUG"
else
  DEBUG="no"
fi
AC_SUBST(DEBUG)

## /local: debug
##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

AX_PD_LATE()
AC_OUTPUT(config/Makefile Makefile)
