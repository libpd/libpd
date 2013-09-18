dnl Process this file with autoconf to produce a configure script.
AC_INIT(iem_adaptfilt.c)

LIBNAME=iem_adaptfilt


dnl Checks for programs.
AC_PROG_CC

AC_SUBST(STK)
AC_SUBST(DFLAGS)
AC_SUBST(LFLAGS)
AC_SUBST(EXT)
AC_SUBST(LD)
AC_SUBST(STRIP)
AC_SUBST(STRIPFLAGS)
AC_SUBST(IEMMATRIX_VERSION)
AC_SUBST(REFERENCEPATH)
AC_SUBST(PDLIBDIR)
AC_SUBST(INCLUDES)
AC_SUBST(LIBNAME)
AC_SUBST(CONFIGUREFLAGS)
AC_SUBST(BUILDLIBRARY)

## store the flags passed to us
## is there no way to get the flags without quotes?
#CONFIGUREFLAGS=${ac_configure_args}
## and is this solution portable? time will show....
CONFIGUREFLAGS=$(echo ${ac_configure_args} | sed "s/'//g")


AC_ARG_WITH(pdversion, [  --with-pdversion=<ver>  enforce a certain pd-version (e.g. 0.37)])
AC_ARG_WITH(extension, [  --with-extension=<ext>  enforce a certain extension for the dynamic library (e.g. dll)])
AC_ARG_WITH(pdpath,    [  --with-pd=</path/to/pd> where to look for pd-headers and and -libs])
AC_ARG_ENABLE(PIC,     [  --disable-PIC           disable compilation with PIC-flag])
AC_ARG_ENABLE(library,[  --disable-library       split the library into single externals])

if test "xno" != "x${enable_library}" ; then
dnl LATER: find a more generic way to generate the .._LIBRARY define
 BUILDLIBRARY="-DBUILD_LIBRARY"
fi

dnl Checks for libraries.
dnl Replace `main' with a function in -lc:
AC_CHECK_LIB(c, main)
AC_CHECK_LIB(crtdll, fclose)

dnl Replace `main' with a function in -lm:
AC_CHECK_LIB(m, main)
dnl Replace `main' with a function in -lpthread:
dnl AC_CHECK_LIB(pthread, main)
dnl Replace `main' with a function in -lstk:
dnl AC_CHECK_LIB(stk, main, STK=yes)

if test "x$with_pd" != "x"; then
 if test -d "${with_pd}/src"; then
  INCLUDES="-I${with_pd}/src ${INCLUDES}"
 fi
 if test -d "${with_pd}/bin"; then
  LIBS="-L${with_pd}/bin ${LIBS}"
 fi
fi

if test "x$includedir" != "x"; then
 for id in $includedir
 do
   if test -d $id; then INCLUDES="-I$id $INCLUDES"; fi
 done
fi
if test "x$libdir" != "x"; then
 for id in $libdir
 do
   if test -d $id; then LIBS="-L$id $LIBS"; fi
 done
fi

AC_CHECK_LIB(pd, nullfn)

dnl Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(stdlib.h stdio.h string.h math.h time.h sys/time.h)

dnl Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_TIME

dnl Checks for library functions.
AC_FUNC_MMAP
AC_CHECK_FUNCS(select socket strerror)


### make-depend flags
if test "x$ac_cv_c_compiler_gnu" = "xyes"; then
 AC_SUBST(MAKEDEP_FLAGS, "-MM")
else
 AC_SUBST(MAKEDEP_FLAGS, "-M")
fi

dnl check for "-mms-bitfields" cflag
dnl why is there no generic compiler-check for a given flag ?
dnl it would make things so easy: AC_CHECK_FLAG([-mms-bitfields],,)
AC_MSG_CHECKING("ms-bitfields")
cat > conftest.c << EOF
int main(){
  return 0;
}
EOF
if ${CC} ${INCLUDES} ${DFLAGS} -o conftest.o conftest.c ${CFLAGS} -mms-bitfields > /dev/null 2>&1
then
  echo "yes"
  CFLAGS="${CFLAGS} -mms-bitfields"
else
  echo "no"
fi




dnl isn't there a better way to check for good linker/stripper ?

dnl if we don't have $LD set, we set it to $(CC)
dnl LD=${LD:=$CC}
if test "x$LD" = "x"
then
 if test "x$host" != "x"
 then
  LD=${host}-ld
  if $(which ${LD} > /dev/null)
  then
    :
  else
    LD=""
  fi
 fi
fi
LD=${LD:=$CC}

dnl if we don't have $STRIP set, we set it to ${host}-strip or strip
AC_CHECK_TOOL([STRIP], [strip], [true])
AC_MSG_CHECKING([if strip is GNU strip])
if $STRIP -V 2>&1 | grep GNU > /dev/null
then
    AC_SUBST(STRIPFLAGS, "--strip-unneeded")
    AC_MSG_RESULT([yes])
else
    AC_SUBST(STRIPFLAGS,"-x")
    AC_MSG_RESULT([no])
fi

DFLAGS=""


if test "x$enable_PIC" != "xno"; then
AC_MSG_CHECKING("PIC")
cat > conftest.c << EOF
int main(){
  return 0;
}
EOF
if ${CC} ${INCLUDES} ${DFLAGS} -o conftest.o conftest.c ${CFLAGS} -fPIC > /dev/null 2>&1
then
  echo "yes"
  CFLAGS="${CFLAGS} -fPIC"
else
  echo "no"
fi
fi


dnl
dnl  OK, checks for machines are here now
dnl
if test `uname -s` = Linux; 
then
  LFLAGS="-export_dynamic  -shared"
  CFLAGS="$CFLAGS"
  EXT=pd_linux	
fi

dnl This should use '-bundle_loader /path/to/pd/bin/pd' instead of'-undefined suppress'
dnl then strip might do something
if test `uname -s` = Darwin; 
then
  LD=cc
  LFLAGS="-bundle -undefined suppress -flat_namespace"
  EXT=pd_darwin
fi

if test `uname | sed -e 's/^MINGW.*/NT/'` = NT; 
then
  LD=gcc
  INCLUDES="-I@prefix@/src"
  DFLAGS="-D__WIN32__"
  LFLAGS="-shared @prefix@/bin/pd.dll"
  EXT=dll
else
  PDLIBDIR="/lib/pd"
fi

if test `uname -s` = IRIX64; 
then
  LFLAGS="-n32 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
        -shared -rdata_shared"
  EXT=pd_irix6
  dnl DFLAGS="-DUNIX -DIRIX6"
fi

if test `uname -s` = IRIX32; 
then
  LFLAGS="-o32 -DUNIX -DIRIX -O2 
         -shared -rdata_shared"
  EXT=pd_irix5
  dnl DFLAGS="-DUNIX -DIRIX5"
fi


if test "x$with_extension" != "x"
then
 EXT=$with_extension
fi


dnl Checks for pd-version, to set the correct help-path
AC_MSG_CHECKING("pd\>=0.37")

if test "$with_pdversion" != ""
then
echo -n "($with_pdversion)... "
  PD_VERSION="$with_pdversion"
else
if test "x$cross_compiling" = "xno"
then

cat > conftest.c << EOF
#include <stdio.h>
#include "m_pd.h"
int main(){
  printf("%d.%d\n", PD_MAJOR_VERSION, PD_MINOR_VERSION);
  return 0;
}
EOF

 if $CC $INCLUDES -o conftest.o conftest.c > /dev/null 2>&1
 then
  PD_VERSION=`./conftest.o`
 else
  PD_VERSION=""
 fi
  echo -n $PD_VERSION
else
dnl we are cross-compiling...
 echo -n "(X)..."
 PD_VERSION="0.38"
fi
fi

let PD_MAJORVERSION=`echo $PD_VERSION | cut -d"." -f1`+0
let PD_MINORVERSION=`echo $PD_VERSION | cut -d"." -f2`+0



if test "$PD_MAJORVERSION" -gt 0 || test "$PD_MINORVERSION" -ge 37
then
  REFERENCEPATH=extra/
  echo " yes"
else
  REFERENCEPATH=doc/5.reference/
  echo " no"
fi

AC_OUTPUT(Make.config)

rm -f conftest.*
