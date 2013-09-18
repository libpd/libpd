

#g++ -O6 -march=pentium -mtune=pentiumpro -I/home/zmoelnig/halcon//include -I/home/zmoelnig/halcon//include/cpp  ./ean13.cpp -I/home/zmoelnig/halcon//include -I/home/zmoelnig/halcon//include/cpp \
#          -L/usr/X11R6/lib -lXext -lX11 -lpthread -lm -ldl -L/home/zmoelnig/halcon//lib/x86-linux2.4-gcc40/ -lhalconcpp -lhalcon -o ../bin/x86-linux2.4-gcc40/ean13

## CFLAGS: -I${HALCONROOT}/include -I${HALCONROOT}/include/cpp
## LDFLAGS: -L${HALCONROOT}/lib/${HALCONARCH} -lhalconcpp -lhalcon


AC_DEFUN([GEM_CHECK_HALCON],
[
AC_ARG_VAR([HALCONROOT], [root-directory where you installed HALCON (override this with '--with-halcon=${HALCONROOT}'])dnl
AC_ARG_VAR([HALCONARCH], [architecture for you HALCON-installation (e.g. 'x86-linux2.4-gcc40'])dnl
AC_ARG_WITH([halcon],
             AC_HELP_STRING([--with-halcon], [enable HALCON video capturing (overrides $HALCONROOT)]))
AC_ARG_WITH([halconarch],
             AC_HELP_STRING([--with-halconarch], [set halcon-arch (overrides $HALCONARCH]))

have_halcon="no"
if test "x$with_halcon" != "xno"; then
  if test -d "${with_halcon}" ; then
    HALCONROOT=${with_halcon}
  fi

  if test "x$with_halconarch" != "x"; then
    HALCONARCH=${with_halconarch}
  fi

  tmp_halcon_includes=""
  if test -d "${HALCONROOT}/include" ; then
   AC_LIB_APPENDTOVAR([tmp_halcon_includes], "-I${HALCONROOT}/include")
  fi
  if test -d "${HALCONROOT}/include/cpp" ; then
   AC_LIB_APPENDTOVAR([tmp_halcon_includes], "-I${HALCONROOT}/include/cpp")
  fi

  if test -d "${HALCONROOT}/lib/${HALCONARCH}" ; then
   tmp_halcon_ldflags="-L${HALCONROOT}/lib/${HALCONARCH}"
  fi
  tmp_halcon_ldflags="-lhalcon ${tmp_halcon_ldflags}"

  tmp_halcon_cppflags_org="$CPPFLAGS"
  tmp_halcon_libs_org="$LIBS"

  CPPFLAGS="$CPPFLAGS $tmp_halcon_includes"
  LIBS="-lm"

  AC_CHECK_HEADER(HalconCpp.h,
                  [
                    have_halcon="yes"
                  ],[
                    have_halcon="no"
                  ])  
  if test "x$have_halcon" = "xyes"; then
   AC_CHECK_LIB(halconcpp, main, , [have_halcon="no"], ["${tmp_halcon_ldflags}"])
  fi

  CPPFLAGS="$tmp_halcon_cppflags_org"
  LIBS="$tmp_halcon_libs_org"

  if test "x$have_halcon" = "xyes"; then
    AC_DEFINE([HAVE_HALCON], [1], [video capturing using MVtec's HALCON])
    GEM_HALCON_CXXFLAGS="${tmp_halcon_includes}"
    GEM_HALCON_LIBS="-lhalconcpp ${tmp_halcon_ldflags}"
  fi

  AC_MSG_CHECKING([for HALCON])
  AC_MSG_RESULT([$have_halcon])

fi

AM_CONDITIONAL(HAVE_HALCON, test x$have_halcon = xyes)

AC_SUBST(GEM_HALCON_CXXFLAGS)
AC_SUBST(GEM_HALCON_LIBS)

])
