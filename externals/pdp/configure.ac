AC_INIT(system/pdp.c)
AC_CONFIG_HEADER(include/pdp_config.h)
AC_PROG_CC
AC_HEADER_STDC


dnl TAG REPO WHEN RELEASE VERSION CHANGES !!!
dnl After changing version, do "make mrproper ; sh bootstrap"

PDP_VERSION=0.12.7




AC_SUBST(PDP_VERSION)

AC_ARG_ENABLE(mmx,    
	[  --enable-mmx            enable MMX support (no)], , enable_mmx=no)

AC_ARG_ENABLE(quicktime,    
	[  --enable-quicktime      enable libquicktime support (yes)], , enable_quicktime=yes)

AC_ARG_ENABLE(v4l,    
	[  --enable-v4l            enable Video4Linux support (yes)], , enable_v4l=yes)

AC_ARG_ENABLE(pwc,    
	[  --enable-pwc            force additional Philips WebCam support (auto)],,enable_pwc=auto)

AC_ARG_ENABLE(x,    
	[  --enable-x              enable X11 support (required by XVideo and GLX) (yes)], , enable_x=yes)

AC_ARG_ENABLE(xv,    
	[  --enable-xv             enable XVideo display support (yes)], , enable_xv=yes)

AC_ARG_ENABLE(sdl,    
	[  --enable-sdl            enable SDL display support (yes)], , enable_sdl=yes)

AC_ARG_ENABLE(glx,    
	[  --enable-glx            enable GLX display support (yes)], , enable_glx=yes)

AC_ARG_ENABLE(gsl,    
	[  --enable-gsl            enable gsl support (for matrix packets) (yes)], , enable_gsl=yes)

AC_ARG_ENABLE(png,    
	[  --enable-png            enable png image load/save support (yes)], , enable_png=yes)

AC_ARG_ENABLE(debug,    
	[  --enable-debug          enable debugging support (no)], , enable_debug=no)


# long double? -Wno-long-double
PDP_CFLAGS="-DPD -Wall -W -Wstrict-prototypes -Wno-unused -Wno-parentheses -Wno-switch -fPIC"
dnl -Werror -Wshadow

dnl the -Wshadow option is way too strict.. gl headers won't compile



dnl setup debugging
if test $enable_debug == yes
then
	AC_DEFINE(PDP_DEBUG, 1, "enable debugging support")
	PDP_CFLAGS="$PDP_CFLAGS -g -Werror"
else
	AC_DEFINE(PDP_DEBUG, 0, "disable debugging support")	
	PDP_CFLAGS="$PDP_CFLAGS -O2 -funroll-loops -fomit-frame-pointer -ffast-math"
fi

if test $prefix == NONE;
then
	prefix=/usr/local
fi


dnl setup forced pwc support
if test $enable_pwc == yes;
then
	AC_DEFINE(HAVE_PWCV4L, 1,enable forced pwc v4l support)
fi


dnl try to locate the pd header in case the setup is nonstandard
dnl check in  $prefix/pd/src then ../pd/src 
dnl if this fails we trust it is in the standard include path
PWD=`pwd`
if test -f $prefix/pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$prefix/pd/src"

elif test -f $prefix/src/pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$prefix/src/pd/src"
elif test -f $PWD/../pd/src/m_pd.h;
then
	PD_CPPFLAGS="-I$PWD/../pd/src"
elif test -f $PWD/../src/m_pd.h;
then
	PD_CPPFLAGS="-I$PWD/../src"
elif test -f /usr/local/include/pd/m_pd.h;
then
	PD_CPPFLAGS="-I/usr/local/include/pd"
elif test -f /usr/include/pd/m_pd.h;
then
	PD_CPPFLAGS="-I/usr/include/pd"
fi


CPPFLAGS="$CPPFLAGS $PD_CPPFLAGS"
AC_CHECK_HEADER(m_pd.h,,
	echo "WARNING: m_pd.h not found. Is PD installed?"
	echo "WARNING: if you have changed PD_CPPFLAGS in Makefile.config.in you can ignore this warning." )

AC_CHECK_LIB(m,sin)

ARCH=`uname -s`
if test $ARCH == Linux;
then
	PDP_LIBRARY_NAME=pdp.pd_linux

	if test $enable_mmx == yes;
	then
		PDP_TARGET=linux_mmx
	else
		PDP_TARGET=linux
	fi
elif test $ARCH == Darwin;
then
	PDP_LIBRARY_NAME=pdp.pd_darwin
	PDP_TARGET=darwin
else
	echo WARNING: Architecture `uname -s` not supported.
	exit
fi



dnl Darwin specific stuff: this is still pretty experimental
dnl How to test if frameworks are present ????
if test $ARCH == Darwin
then
	PD_EXECUTABLE=$prefix/bin/pd
	LIBS="$LIBS -L/sw/lib"
	CPPFLAGS="$CPPFLAGS -I/sw/include"
	PDP_EXTRA_CPPFLAGS="-I/sw/include"

dnl if not darwin, assume target is linux
else
  have_v4l="no"
  have_libv4l="no"
  AC_CHECK_HEADERS([libv4l1-videodev.h], have_v4l="yes")
  AC_CHECK_HEADERS([linux/videodev.h], have_libv4l="yes")

  if test "x$have_libv4l" = "xyes"
  then
    have_v4l="yes"
  fi

  if test "x$enable_v4l" = "xno"
  then
   have_v4l="no"
  fi

  if test "x$have_v4l" = "xyes"
  then
		PDP_OPTMOD="$PDP_OPTMOD pdp_v4l.o"
		AC_DEFINE(HAVE_PDP_V4L, 1, [build pdp_v4l])

    if test "x$have_libv4l" = "xyes"
    then
  		AC_DEFINE(HAVE_PDP_LIBV4L, 1, [build pdp_v4l with libv4l])
      PDP_V4L_CFLAGS="`pkg-config --cflags libv4l 2>/dev/null || pkg-config --cflags libv4l2`"
      PDP_V4L_LIBS="`pkg-config --libs libv4l 2>/dev/null || pkg-config --libs libv4l2`"
    else
      PDP_V4L_CFLAGS=""
      PDP_V4L_LIBS=""
    fi
    PDP_CFLAGS="$PDP_CFLAGS $PDP_V4L_CFLAGS"
    LIBS="$LIBS $PDP_V4L_LIBS"
  else
    AC_MSG_RESULT([not building pdp_v4l])
  fi
fi

dnl optional gsl support
if test $enable_gsl == yes;
then
  AC_CHECK_LIB(gslcblas,main)
  AC_CHECK_LIB(gsl,main,
    PDP_OPTTYPES="$PDP_OPTTYPES pdp_matrix.o"
    PDP_MATRIX_BASIC="$PDP_MATRIX_BASIC pdp_mat_mul.o pdp_mat_lu.o pdp_mat_vec.o"
    PDP_IMAGE_BASIC="$PDP_IMAGE_BASIC pdp_cheby.o"
    PDP_IMAGE_SPECIAL="$PDP_IMAGE_SPECIAL pdp_histo.o"
    AC_DEFINE(HAVE_PDP_GSL, 1, gsl support)
    PDP_GSL_FOUND=1,
    echo "   libgsl not found: not building matrix type support")

  if test -n "$PDP_GSL_FOUND"; then
    if test $ARCH == Darwin
    then
      LIBS="$LIBS -lgslcblas -lgsl"
    else
      dnl if not darwin, assume target is linux
      LIBS="$LIBS `pkg-config --libs gsl`"
    fi
  fi
fi


dnl optional png support
if test $enable_png == yes
then
	AC_CHECK_LIB(png, png_read_image,
		LIBS="$LIBS -lz -lpng"
		AC_DEFINE(HAVE_PDP_PNG, 1, build png support),
		echo "   libpng not found: not building png support")
fi


dnl optional xwindow support
if test $enable_x == yes;
then
	AC_CHECK_LIB(X11, XOpenDisplay,
		PDP_X11MOD="$PDP_X11MOD pdp_xwindow.o"
		AC_DEFINE(HAVE_PDP_X, 1, build X11 support)
		LIBS="$LIBS -L/usr/X11R6/lib -lX11",
		echo "   libX11 not found: not building X11 support",
		-L/usr/X11R6/lib)

else
	dnl no x support -> no xv and glx support
	enable_xv=no
	enable_glx=no

fi

dnl optional xvideo support
if test $enable_xv == yes;
then
	AC_CHECK_LIB(Xv, XvPutImage,
		PDP_X11MOD="$PDP_X11MOD pdp_xvideo.o"
		PDP_OPTMOD="$PDP_OPTMOD pdp_xv.o"
		LIBS="$LIBS -lXv -lXext"
		AC_DEFINE(HAVE_PDP_XV, 1, build pdp_xv),
		echo "   libXv not found: not building pdp_xv",
		-L/usr/X11R6/lib -lX11 -lXext)
fi

dnl optional libquicktime support
if test $enable_quicktime == yes;
then
	AC_CHECK_LIB(quicktime, lqt_decode_video,
		PDP_OPTMOD="$PDP_OPTMOD pdp_qt.o"
		LIBS="$LIBS -lquicktime"
		AC_DEFINE(HAVE_PDP_QT, 1, build pdp_qt),
		echo "   libquicktime not found: not building pdp_qt")
fi


dnl optional glx support
if test $enable_glx == yes
then
	AC_CHECK_LIB(GL, glXSwapBuffers,
		PDP_OPTMOD="$PDP_OPTMOD pdp_glx.o"
		LIBS="$LIBS -lGL -lGLU"
		AC_DEFINE(HAVE_PDP_GLX, 1, build pdp_glx),
		echo "   libGL not found: not building pdp_glx",
		-L/usr/X11R6/lib -lGLU)
fi


dnl optional sdl support
if test $enable_sdl == yes
then
	AC_CHECK_LIB(SDL, SDL_Init, 
		PDP_OPTMOD="$PDP_OPTMOD pdp_sdl.o"
		LIBS="$LIBS -lSDL"
		AC_DEFINE(HAVE_PDP_SDL, 1, build pdp_sdl),
		echo "   libSDL not found: not building pdp_sdl")
fi


echo target is $PDP_TARGET

echo "used configure options:"
echo "  --enable-mmx=$enable_mmx"
echo "  --enable-quicktime=$enable_quicktime"
echo "  --enable-v4l=$enable_v4l"
echo "  --enable-pwc=$enable_pwc"
echo "  --enable-sdl=$enable_sdl"
echo "  --enable-x=$enable_x"
echo "  --enable-xv=$enable_xv"
echo "  --enable-glx=$enable_glx"
echo "  --enable-gsl=$enable_gsl"
echo "  --enable-png=$enable_png"
echo "  --enable-debug=$enable_debug"


AC_SUBST(PD_CPPFLAGS)
AC_SUBST(PD_EXECUTABLE)
AC_SUBST(PDP_EXTRA_CPPFLAGS)
AC_SUBST(PDP_CFLAGS)
AC_SUBST(PDP_EXTRA_CFLAGS)
AC_SUBST(PDP_LIBRARY_NAME)
AC_SUBST(PDP_TARGET)

AC_SUBST(PDP_OPTMOD)
AC_SUBST(PDP_X11MOD)
AC_SUBST(PDP_PDMOD)
AC_SUBST(PDP_OPTTYPES)

AC_SUBST(PDP_IMAGE_BASIC)
AC_SUBST(PDP_IMAGE_SPECIAL)
AC_SUBST(PDP_MATRIX_BASIC)


AC_CONFIG_FILES(Makefile.config)
AC_CONFIG_FILES(bin/pdp-config)
AC_OUTPUT



