AC_INIT(pdp/pdp_ca_system.c)
AC_PROG_CC
AC_HEADER_STDC

dnl default install prefix is /usr/local
if test $prefix == "NONE";
then
	prefix=/usr/local
fi

AC_PATH_PROG(PDP_CONFIG,pdp-config,"no", $PATH)

AC_CHECK_LIB(m,sin)
AC_CHECK_LIB(dl,dlopen,, echo libdl not found. sorry... || exit 1)


TOPSRC=`pwd`
PARENT=`dirname $TOPSRC`

dnl if pdp-config is found use it to get the cflags
if ! test $PDP_CONFIG == "no"
then
	PDP_CPPFLAGS=`$PDP_CONFIG --cflags`

dnl if not, check in the parent dir (in case we are distributed with the pdp package)
elif test -f $PARENT/include/pdp.h
then
	PDP_CPPFLAGS="-I$PARENT/include"
fi

CPPFLAGS="$CPPFLAGS $PDP_CPPFLAGS"
AC_CHECK_HEADER(m_pd.h,,PD_OK=no)
AC_CHECK_HEADER(pdp.h,,PDP_OK=no)

if test PD_OK == "no";
then
	echo "WARNING: m_pd.h not found. Is PD installed?
	echo "WARNING: You can ignore this warning if you have set the PD_CFLAGS manually in Makefile.config.in
fi

if test PDP_OK == "no";
then
	echo "WARNING: pdp.h not found. Is PDP installed?
	echo "WARNING: You can ignore this warning if you have set the PDP_CFLAGS manually in Makefile.config.in
fi

CPPFLAGS="$CPPFLAGS $PDFLAGS $PDPFLAGS -I$TOPSRC/include"

DEFAULT_RULES_LIB=$prefix/lib/scaf/default.scafo;
AC_SUBST(DEFAULT_RULES_LIB)

AC_CONFIG_FILES(Makefile.config)
AC_OUTPUT
