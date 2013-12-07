dnl Process this file with autoconf to produce a configure script.
AC_INIT(iem16.c)


dnl Checks for programs.
AC_PROG_CC

AC_SUBST(STK)
AC_SUBST(DFLAGS)
AC_SUBST(LFLAGS)
AC_SUBST(EXT)
AC_SUBST(LD)
AC_SUBST(STRIP)
AC_SUBST(STRIPFLAGS)
AC_SUBST(LIBRARY_NAME)
AC_SUBST(LIBRARY_VERSION)
AC_SUBST(BUILDLIBRARY)
AC_SUBST(REFERENCEPATH)
AC_SUBST(PDLIBDIR)
AC_SUBST(INCLUDES)
AC_SUBST(CONFIGUREFLAGS)

LIBRARY_NAME=${ac_unique_file%.*}

## store the flags passed to us
## is there no way to get the flags without quotes?
#CONFIGUREFLAGS=${ac_configure_args}
## and is this solution portable? time will show....
CONFIGUREFLAGS=$(echo ${ac_configure_args} | sed "s/'//g")

AC_ARG_WITH(extension, [  --with-extension=<ext>  enforce a certain extension for the dynamic library (e.g. dll)])
AC_ARG_WITH(pdpath,    [  --with-pd=</path/to/pd> where to look for pd-headers and and -libs])

AC_ARG_ENABLE(PIC,     [  --disable-PIC           disable compilation with PIC-flag])
if test "x" = "x${enable_PIC}" ; then
 enable_PIC="${with_PIC}"
fi

if test "x$enable_PIC" != "xno"; then
 AC_CHECK_CFLAGS([-fPIC])
fi


AC_ARG_ENABLE(library,[  --disable-library       split the library into single externals])
if test "xno" != "x${enable_library}" ; then
dnl LATER: find a more generic way to generate the .._LIBRARY define
 BUILDLIBRARY="-DLIBRARY"
fi


dnl check for "-mms-bitfields" cflag
dnl it would make things so easy: AC_CHECK_FLAG([-mms-bitfields],,)
AC_CHECK_CFLAGS([-mms-bitfields])


dnl Checks for libraries.
dnl Replace `main' with a function in -lc:
AC_CHECK_LIB(c, main)
AC_CHECK_LIB(crtdll, fclose)
AC_CHECK_LIB(coldname, fdopen)

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
if test "x$STRIP" = "x"
then
 if test "x$host" != "x"
 then
  STRIP=${host}-strip
  if $(which ${host}-strip > /dev/null)
  then
    :
  else
    STRIP="echo fake strip"
  fi
 else
  STRIP=strip
 fi
fi
dnl STRIP=${STRIP:=strip}


dnl
dnl  OK, checks which machines are here now
dnl this needs some rethinking when cross-compiling (?)
dnl
AC_CHECK_LDFLAGS([-Wl,--export-dynamic -shared])

if test `uname -s` = Linux; 
then
  EXT=pd_linux	
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_icc" = "yes"; then
    CC=icc
  fi

  if test $CC = "icc"
  then
    LD=$CC
    AC_CHECK_LDFLAGS([-ip -ipo_obj])
    AC_CHECK_CFLAGS([-ip -ipo_obj])
  fi
fi

dnl This should use '-bundle_loader /path/to/pd/bin/pd' instead of'-undefined suppress'
dnl then strip might do something
if test `uname -s` = Darwin; 
then
  LD=${CC}
  EXT=pd_darwin
  STRIP="echo faking strip"
  STRIPFLAGS=

  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi

#AC_CHECK_LDFLAGS([-dynamiclib -mmacosx-version-min=10.3 -undefined dynamic_lookup],
#                  ,
	          AC_CHECK_LDFLAGS([-bundle -undefined suppress -flat_namespace])
#                 )

if test `uname | sed -e 's/^MINGW.*/NT/'` = NT ; 
then
  LD=${CC}
#  LDFLAGS="-shared pd.dll"
  EXT=dll
fi

AC_CHECK_LDFLAGS([-shared pd.dll])

if test `uname -s` = IRIX64; 
then
  LDFLAGS="-n32 -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
        -shared -rdata_shared"
  EXT=pd_irix6
  dnl DFLAGS="-DUNIX -DIRIX6"
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi

if test `uname -s` = IRIX32; 
then
  LDFLAGS="-o32 -DUNIX -DIRIX -O2 
         -shared -rdata_shared"
  EXT=pd_irix5
  dnl DFLAGS="-DUNIX -DIRIX5"
  STRIPFLAGS="--strip-unneeded"
  if test "$enable_lpt" = "yes"; then
    AC_MSG_ERROR("lpt not supported on this platform");
  fi
  if test "$enable_lpt" = ""; then
    AC_MSG_WARN("lpt not supported on this platform - disabling");
    enable_lpt="no";
  fi
fi

if test "x$with_extension" != "x"
then
 EXT=$with_extension
fi

dnl check for ${LIBRARY_NAME}-version (but why...)
AC_MSG_CHECKING("library version")

if test "$with_version" != ""
then
  echo -n "($with_version)... "
  LIBRARY_VERSION="$with_version"
else

if test "x$cross_compiling" = "xno"
then
cat > conftest.c << EOF
#include <stdio.h>
#include "${LIBRARY_NAME}.h"
int main(){
  printf("%s\n", VERSION);
  return 0;
}
EOF

if ${CC} ${INCLUDES} -o conftest.o conftest.c > /dev/null 2>&1
then
  LIBRARY_VERSION=`./conftest.o`
  echo "$LIBRARY_VERSION"
else
  LIBRARY_VERSION=""
  echo "(unknown)"
fi
else
  LIBRARY_VERSION="X"
  echo "(X)"
fi
fi

LFLAGS=${LDFLAGS}
AC_OUTPUT(Make.config)

dnl rm -f conftest.*
