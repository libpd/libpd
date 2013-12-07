AC_INIT(modules/setup.c)
AC_PROG_CC
AC_HEADER_STDC


echo -n "checking architecture... "
ARCH=`uname -s`
if test $ARCH == Linux;
then
	PD_INCLUDE_DIR=$prefix/pd/src
	LIBNAME=creb.pd_linux
	LIBFLAGS="-export_dynamic -shared"
	echo "Linux"

elif test $ARCH == Darwin;
then
	PD_INCLUDE_DIR=$prefix/pd/src
	LIBNAME=creb.pd_darwin
	LIBFLAGS="-bundle -bundle_loader $prefix/pd/bin/pd"
	echo "Darwin"

else
	echo WARNING: Architecture `uname -s` not supported.
	exit;
fi


CPPFLAGS="$CPPFLAGS -I$prefix/pd/src"

AC_CHECK_HEADER(m_pd.h,,
	echo "WARNING: m_pd.h not found in the standard include path."
	echo "         If PD_DIR in Makefile.config.in is set correctly you can ignore this warning." )

AC_CHECK_LIB(m,sin,,
	echo "Math library not found. sorry..." || exit)


AC_SUBST(PD_INCLUDE_DIR)
AC_SUBST(LIBNAME)
AC_SUBST(LIBFLAGS)
AC_CONFIG_FILES(Makefile.config)
AC_OUTPUT
