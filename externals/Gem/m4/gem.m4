dnl Copyright (C) 2005 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.
# GEM_ARG_WITH(WITHARG, [TEXT], [FORCE])
# enables the "--with-WITHARG" flag; if FORCE is non-empty and no value is
# set by the user, the default value "yes" is assigned to with_WITHARG
# --------------------------------------------------------------
AC_DEFUN([GEM_ARG_WITH],
[
  if test "x$with_ALL" = "xyes" && test "x${with_$1}" = "x"; then with_$1="yes"; fi 
  if test "x$with_ALL" = "xno"  && test "x${with_$1}" = "x"; then with_$1="no"; fi

  AC_ARG_WITH([$1],
             AC_HELP_STRING([--without-$1], [disable $1-lib ($2)]),,[
                if test "x$3" != "x"; then with_$1="yes"; fi
           ])
])# GEM_ARG_WITH
# inverse of GEM_ARG_WITH
AC_DEFUN([GEM_ARG_WITHOUT],
[AC_ARG_WITH([$1],
             AC_HELP_STRING([--with-$1], [enable $1-lib ($2)]),,[
                if test "x$3" = "xforce"; then with_$1="no"; fi
           ])
])# GEM_ARG_WITHOUT

# same as GEM_ARG_WITH but with "enable"
AC_DEFUN([GEM_ARG_ENABLE],
[AC_ARG_ENABLE([$1],
             AC_HELP_STRING([--disable-$1], [disable $1 ($2)]),,[
                if test "x$3" != "x"; then enable_$1="yes"; fi
             ])
])# GEM_ARG_ENABLE

# inverse of GEM_ARG_ENABLE
AC_DEFUN([GEM_ARG_DISABLE],
[AC_ARG_ENABLE([$1],
             AC_HELP_STRING([--enable-$1], [enable $1 ($2)]),,[
                if test "x$3" != "x"; then enable_$1="no"; fi
             ])
])# GEM_ARG_DISABLE


AC_DEFUN([GEM_TARGET], [
define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                             [ABCDEFGHIJKLMNOPQRSTUVWXYZ____])])
AC_CONFIG_FILES([src/$1/Makefile])

AC_ARG_ENABLE([$1],
             AC_HELP_STRING([--disable-$1], [disable $1-objects]),
             [
                if test "x$enableval" != "xno"; then
                  AC_MSG_RESULT([building Gem with $1-objects])
                  target_[]NAME="yes"
                else
                   AC_MSG_RESULT([building Gem without $1-objects])
                  target_[]NAME="no"
                fi
             ],
             [
                  AC_MSG_RESULT([building Gem with $1-objects])
                  target_[]NAME="yes"
             ])
AM_CONDITIONAL(TARGET_[]NAME, [test "x$target_[]NAME" != "xno"])
undefine([NAME])
])# GEM_TARGET

AC_DEFUN([GEM_TARGET_DISABLED], [
define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                             [ABCDEFGHIJKLMNOPQRSTUVWXYZ____])])

AC_CONFIG_FILES([src/$1/Makefile])
AC_ARG_ENABLE([$1],
             AC_HELP_STRING([--enable-$1], [enable $1-objects]),
             [
                if test "x$enableval" != "xyes"; then
                   AC_MSG_RESULT([building Gem without $1-objects])
                  target_[]NAME="no"
                else
                  AC_MSG_RESULT([building Gem with $1-objects])
                  target_[]NAME="yes"
                fi
             ],
             [
                   AC_MSG_RESULT([building Gem without $1-objects])
                   target_[]NAME="no"
             ])
AM_CONDITIONAL(TARGET_[]NAME, [test "x$target_[]NAME" != "xno"])
undefine([NAME])
])# GEM_TARGET_DISABLED



# GEM_CHECK_LIB(NAME, LIBRARY, FUNCTION, [ACTION-IF-FOUND], [ACTION-IF-NOT-FOUND], [ADDITIONAL_LIBS], [HELP-TEXT], [DEFAULT-WITH_VALUE])
#
## this automatically adds the option "--without-NAME" to disable the checking for the library
## additionally it gives the options "--with-NAME-includes" and "--with-NAME-libs", 
## to manually add searchpaths for the include-files and libraries
## the only check performed on these paths is whether they are real paths
AC_DEFUN([GEM_CHECK_LIB],
[
 define([Name],[translit([$1],[./-+], [____])])
 define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                              [ABCDEFGHIJKLMNOPQRSTUVWXYZ____])])
AC_SUBST(GEM_LIB_[]NAME[]_CFLAGS)
AC_SUBST(GEM_LIB_[]NAME[]_LIBS)

AC_ARG_WITH([Name],
             AC_HELP_STRING([--without-[]Name], [disable Name ($7)]))
AC_ARG_WITH([]Name-includes,
             AC_HELP_STRING([--with-[]Name-includes=/path/to/[]Name/include/], [include path for Name]))
AC_ARG_WITH([]Name-libs,
             AC_HELP_STRING([--with-[]Name-libs=/path/to/[]Name/lib/], [library path for Name]))

  if test "x$with_[]Name" = "x"; then with_[]Name="$8"; fi 

  if test "x$with_ALL" = "xyes" && test "x$with_[]Name" = "x"; then with_[]Name="yes"; fi 
  if test "x$with_ALL" = "xno"  && test "x$with_[]Name" = "x"; then with_[]Name="no"; fi

tmp_gem_check_lib_cppflags="$CPPFLAGS"
tmp_gem_check_lib_cflags="$CFLAGS"
tmp_gem_check_lib_cxxflags="$CXXFLAGS"
tmp_gem_check_lib_ldflags="$LDFLAGS"
tmp_gem_check_lib_libs="$LIBS"

if test x$with_[]Name = "xno"; then
  have_[]Name="no (forced)"
else
  if test -d "$with_[]Name[]_includes"; then
    CFLAGS="-I$with_[]Name[]_includes $CFLAGS"
    CPPFLAGS="-I$with_[]Name[]_includes $CPPFLAGS"
    CXXFLAGS="-I$with_[]Name[]_includes $CXXFLAGS"
  fi
  if test -d "$with_[]Name[]_libs"; then
    LIBS="-L$with_[]Name[]_libs $LIBS"
  fi
  AS_LITERAL_IF([$2],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$2_$3])],
              [AS_VAR_PUSHDEF([ac_Lib], [ac_cv_lib_$2''_$3])])dnl

## unset ac_Lib is possible
  (unset ac_Lib) >/dev/null 2>&1 && unset ac_Lib

## 1st we check, whether pkg-config knows something about this package
dnl  PKG_CHECK_MODULES(AS_TR_CPP(PKG_$1), $1,AS_VAR_SET(acLib)yes, AC_CHECK_LIB([$2],[$3],,,[$6]))
  PKG_CHECK_MODULES(AS_TR_CPP(PKG_$1), $1,AS_VAR_SET(ac_Lib)yes,:)

  if test "x$ac_Lib" != "xyes"; then
## pkg-config has failed
## check whether there is a ${1}-config lying around
   AC_MSG_CHECKING([for $1-config])
   gem_check_lib_pkgconfig_[]NAME=""
   if test "x$1" != "x"; then
    gem_check_lib_pkgconfig_[]NAME="$1"-config
    if which -- "$gem_check_lib_pkgconfig_[]NAME" >/dev/null 2>&1; then
     gem_check_lib_pkgconfig_[]NAME=`which "$1"-config`
     AC_MSG_RESULT([yes])
    else
     gem_check_lib_pkgconfig_[]NAME=""
     AC_MSG_RESULT([no])
    fi
   fi

   if test "x$gem_check_lib_pkgconfig_[]NAME" != "x"; then
## fabulous, we have ${1}-config
## lets see what it reveals

## if PKG_<name>_CFLAGS is undefined, we try to get it from ${1}-config
    if test "x$PKG_[]NAME[]_CFLAGS" = "x"; then
      if $gem_check_lib_pkgconfig_[]NAME --cflags >/dev/null 2>&1; then
        PKG_[]NAME[]_CFLAGS=$(${gem_check_lib_pkgconfig_[]NAME} --cflags)
        PKG_CFLAGS="$PKG_[]NAME[]_CFLAGS $PKG_CFLAGS"
      fi
    fi
 
## if PKG_<name>_LIBS is undefined, we try to get it from ${1}-config
## we first try to get it via "--plugin-libs" (this is almost certainly what we want)
## if that fails we try to get it via  "--libs"
    if test "x$PKG_[]NAME[]_LIBS" = "x"; then
      if $gem_check_lib_pkgconfig_[]NAME --plugin-libs >/dev/null 2>&1; then
        PKG_[]NAME[]_LIBS=$(${gem_check_lib_pkgconfig_[]NAME} --plugin-libs)
      else
       if $gem_check_lib_pkgconfig_[]NAME --libs >/dev/null 2>&1; then
        PKG_[]NAME[]_LIBS=$(${gem_check_lib_pkgconfig_[]NAME} --libs)
       fi
      fi
    fi
   fi

## if we still don't know about the libs, we finally fall back to AC_CHECK_LIB
   if test "x${PKG_[]NAME[]_LIBS}" = "x"; then
    AC_CHECK_LIB([$2],[$3],,,[$6])
    PKG_[]NAME[]_LIBS="-l$2"
   else
     PKG_LIBS="${PKG_[]NAME[]_LIBS} ${PKG_LIBS}"
   fi
  fi

  AS_IF([test "x$ac_Lib" != "xno"],
   [
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$1),[1], [$7])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_LIB$2),[1], [$7])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_$3),[1], [Define to 1 if you have the `$3' function.])
    GEM_LIB_[]NAME[]_CFLAGS=${PKG_[]NAME[]_CFLAGS}
    GEM_LIB_[]NAME[]_LIBS=${PKG_[]NAME[]_LIBS}
dnl    PKG_LIBS="$6 ${PKG_LIBS}"
    have_[]Name="yes"
dnl turn of further checking for this package
    with_[]Name="no"
    [$4]
   ],[
    have_[]Name="no"
    [$5]
   ])
   AS_VAR_POPDEF([ac_Lib])dnl

fi[]dnl

AM_CONDITIONAL(HAVE_LIB_[]NAME, [test "x${have_[]Name}" = "xyes"])
# restore original flags
CPPFLAGS="$tmp_gem_check_lib_cppflags"
CFLAGS="$tmp_gem_check_lib_cflags"
CXXFLAGS="$tmp_gem_check_lib_cxxflags"
LDFLAGS="$tmp_gem_check_lib_ldflags"
LIBS="$tmp_gem_check_lib_libs"


undefine([Name])
undefine([NAME])
])# GEM_CHECK_LIB

# GEM_CHECK_CXXFLAGS(ADDITIONAL-CXXFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(CXX) compiler accepts the ADDITIONAL-CXXFLAGS
# if so, they are added to the CXXFLAGS
AC_DEFUN([GEM_CHECK_CXXFLAGS],
[
  AC_MSG_CHECKING([whether compiler accepts "$1"])
cat > conftest.c++ << EOF
int main(){
  return 0;
}
EOF
if $CXX $CPPFLAGS $CXXFLAGS -o conftest.o conftest.c++ [$1] > /dev/null 2>&1
then
  AC_MSG_RESULT([yes])
  CXXFLAGS="${CXXFLAGS} [$1]"
  [$2]
else
  AC_MSG_RESULT([no])
  [$3]
fi
])# GEM_CHECK_CXXFLAGS

# GEM_CHECK_FRAMEWORK(FRAMEWORK, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
#
AC_DEFUN([GEM_CHECK_FRAMEWORK],
[
  define([NAME],[translit([$1],[abcdefghijklmnopqrstuvwxyz./+-],
                              [ABCDEFGHIJKLMNOPQRSTUVWXYZ____])])
  AC_SUBST(GEM_FRAMEWORK_[]NAME[])

  AC_MSG_CHECKING([for "$1"-framework])

  gem_check_ldflags_org="${LDFLAGS}"
  LDFLAGS="-framework [$1] ${LDFLAGS}"

  AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])], [gem_check_ldflags_success="yes"],[gem_check_ldflags_success="no"])

  if test "x$gem_check_ldflags_success" = "xyes"; then
    AC_MSG_RESULT([yes])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_$1), [1], [framework $1])
    AC_DEFINE_UNQUOTED(AS_TR_CPP(HAVE_GEM_FRAMEWORK_$1), [1], [framework $1])
    GEM_FRAMEWORK_[]NAME[]="-framework [$1]"
    [$2]
  else
    AC_MSG_RESULT([no])
    LDFLAGS="$gem_check_ldflags_org"
    [$3]
  fi
AM_CONDITIONAL(HAVE_FRAMEWORK_[]NAME, [test "x$gem_check_ldflags_success" = "xyes"])
undefine([NAME])
])# GEM_CHECK_FRAMEWORK

# GEM_CHECK_LDFLAGS(ADDITIONAL-LDFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(LD) linker accepts the ADDITIONAL-LDFLAGS
# if so, they are added to the LDFLAGS
AC_DEFUN([GEM_CHECK_LDFLAGS],
[
  AC_MSG_CHECKING([whether linker accepts "$1"])
  gem_check_ldflags_org="${LDFLAGS}"
  LDFLAGS="$1 ${LDFLAGS}"

  AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])], [gem_check_ldflags_success="yes"],[gem_check_ldflags_success="no"])

  if test "x$gem_check_ldflags_success" = "xyes"; then
    AC_MSG_RESULT([yes])
    [$2]
  else
    AC_MSG_RESULT([no])
    LDFLAGS="$gem_check_ldflags_org"
    [$3]
  fi
])# GEM_CHECK_LDFLAGS


AC_DEFUN([GEM_CHECK_FAT],
[
AC_ARG_ENABLE(fat-binary,
       [  --enable-fat-binary=ARCHS
                          build an Apple Multi Architecture Binary (MAB);
                          ARCHS is a comma-delimited list of architectures for
                          which to build; if ARCHS is omitted, then the package
                          will be built for all architectures supported by the
                          platform (e.g. "ppc,i386" for MacOS/X and Darwin); 
                          if this option is disabled or omitted entirely, then
                          the package will be built only for the target 
                          platform; when building multiple architectures, 
			  dependency-tracking must be disabled],
       [fat_binary=$enableval], [fat_binary=no])
if test "$fat_binary" != no; then
    AC_MSG_CHECKING([target architectures])

    # Respect TARGET_ARCHS setting from environment if available.
    if test -z "$TARGET_ARCHS"; then
   	# Respect ARCH given to --enable-fat-binary if present.
     if test "$fat_binary" != yes; then
	    TARGET_ARCHS=`echo "$fat_binary" | tr ',' ' '`
     else
	    # Choose a default set of architectures based upon platform.
      TARGET_ARCHS="ppc i386"
     fi
    fi
    AC_MSG_RESULT([$TARGET_ARCHS])

   define([Name],[translit([$1],[./-], [___])])
   # /usr/lib/arch_tool -archify_list $TARGET_ARCHS
   []Name=""
   tmp_arch_count=0
   for archs in $TARGET_ARCHS 
   do
    []Name="$[]Name -arch $archs"
    tmp_arch_count=$((tmp_arch_count+1))
   done

   if test "$tmp_arch_count" -gt 1; then
     if test "x$enable_dependency_tracking" != xno; then
     	AC_MSG_ERROR([when building for multiple architectures, you MUST turn off dependency-tracking])
     fi
   fi

   if test "x$[]Name" != "x"; then
    tmp_arch_cflags="$CFLAGS"
    tmp_arch_cxxflags="$CXXFLAGS"
    GEM_CHECK_CXXFLAGS($[]Name,,[]Name="")
    []Name[]_CXXFLAGS+=$[]Name
    CFLAGS="$tmp_arch_cflags"
    CXXFLAGS="$tmp_arch_cxxflags"
   fi

   if test "x$[]Name" != "x"; then
    tmp_arch_ldflags="$LDFLAGS"
    GEM_CHECK_LDFLAGS($[]Name,,[]Name="")
    []Name[]_LDFLAGS+=$[]Name
    LDFLAGS="$tmp_arch_ldflags"
   fi

   undefine([Name])
fi
])# GEM_CHECK_FAT

# GEM_CHECK_RTE()
#
# checks for RTE (currently: Pd)
# if so, they are added to the LDFLAGS, CFLAGS and whatelse
AC_DEFUN([GEM_CHECK_RTE],
[
ARCH=$(uname -m)
KERN=$(uname -s)
IEM_OPERATING_SYSTEM

AC_SUBST(GEM_RTE_CFLAGS)
AC_SUBST(GEM_RTE_LIBS)

if test "x${libdir}" = "x\${exec_prefix}/lib"; then
 libdir='${exec_prefix}/lib/pd/extra'
fi

tmp_rte_cppflags="$CPPFLAGS"
tmp_rte_cflags="$CFLAGS"
tmp_rte_cxxflags="$CXXFLAGS"
tmp_rte_ldflags="$LDFLAGS"

GEM_RTE_CFLAGS="-DPD"
GEM_RTE_LIBS=""
GEM_RTE="pd"

AC_ARG_WITH([pd], 
	        AS_HELP_STRING([--with-pd=<path/to/pd>],[where to find pd-binary (./bin/pd.exe) and pd-sources]))
AC_ARG_WITH([pdversion], 
		AS_HELP_STRING([--with-pdversion=<ver>],[enforce a certain pd-version (e.g. 0.37)]))

if test -d "$with_pd" ; then
 if test -d "${with_pd}/src" ; then
   AC_LIB_APPENDTOVAR([GEM_RTE_CFLAGS],"-I${with_pd}/src")
 else
   AC_LIB_APPENDTOVAR([GEM_RTE_CFLAGS],"-I${with_pd}")
 fi
 if test -d "${with_pd}/bin" ; then
   GEM_RTE_LIBS="${GEM_RTE_LIBS}${GEM_RTE_LIBS:+ }-L${with_pd}/bin"
 else
   GEM_RTE_LIBS="${GEM_RTE_LIBS}${GEM_RTE_LIBS:+ }-L${with_pd}"
 fi

 CPPFLAGS="$CPPFLAGS ${GEM_RTE_CFLAGS}"
 CFLAGS="$CFLAGS ${GEM_RTE_CFLAGS}"
 CXXFLAGS="$CXXFLAGS ${GEM_RTE_CFLAGS}"
 LIBS="$LIBS ${GEM_RTE_LIBS}"
fi

AC_CHECK_HEADERS(m_pd.h)

if test "x$with_pdversion" != "x"; then
  PD_VERSION="$with_pdversion"
else
AC_MSG_CHECKING([for Pd-version])
cat > conftest.c << EOF
#include <stdio.h>
#include "m_pd.h"
int main(){
  printf("%d.%d\n", PD_MAJOR_VERSION, PD_MINOR_VERSION);
  return 0;
}
EOF
 if $CXX $CFLAGS -o conftest.o conftest.c > /dev/null 2>&1; then
  PD_VERSION=`./conftest.o`
 else
  PD_VERSION=""
 fi
fi

PD_MAJORVERSION=$(( $(echo $PD_VERSION | cut -d"." -f1) ))
PD_MINORVERSION=$(( $(echo $PD_VERSION | cut -d"." -f2) ))

if test "$PD_MAJORVERSION" -gt 0 -o "$PD_MINORVERSION" -ge 37; then
  GEM_RTE_REFERENCEPATH=extra/Gem
else
  GEM_RTE_REFERENCEPATH=doc/5.reference/Gem
fi

AC_MSG_RESULT([${PD_MAJORVERSION}.${PD_MINORVERSION}])

AC_SUBST(GEM_RTE_REFERENCEPATH)

dnl LATER check why this doesn't use the --with-pd includes
dnl for now it will basically disable anything that needs s_stuff.h if it cannot be found in /usr[/local]/include
AC_CHECK_HEADERS([s_stuff.h], [], [],
[#ifdef HAVE_M_PD_H
# define PD
# include "m_pd.h"
#endif
])

### this should only be set if Pd has been found
# the extension
AC_ARG_WITH([extension], 
		AS_HELP_STRING([--with-extension=<ext>],[enforce a certain file-extension (e.g. pd_linux)]))
if test "x$with_extension" != "x"; then
 EXT=$with_extension
else
 EXT=pd_`echo $host_os | sed -e '/.*/s/-.*//' -e 's/\[.].*//'`
  if test "x$KERN" = "xDarwin"; then
    EXT=pd_darwin
  else
   if test "x$host_os" = "x"
   then
    dnl just assuming that it is still linux (e.g. x86_64)
    EXT="pd_linux"
   fi
  fi
fi
GEM_RTE_EXTENSION=$EXT
AC_SUBST(GEM_RTE_EXTENSION)

CPPFLAGS="$tmp_rte_cppflags"
CFLAGS="$tmp_rte_cflags"
CXXFLAGS="$tmp_rte_cxxflags"
LDFLAGS="$tmp_rte_ldflags"
]) # GEM_CHECK_RTE


AC_DEFUN([GEM_CHECK_THREADS],
[
GEM_ARG_ENABLE([threads], [default: use threads])

AC_SUBST(GEM_PTHREAD_CFLAGS)
AC_SUBST(GEM_PTHREAD_LIBS)

if test "x$enable_threads" != "xno"; then
 AX_PTHREAD
 GEM_THREADS_CFLAGS=""
 AC_LIB_APPENDTOVAR([GEM_THREADS_CFLAGS], "${PTHREAD_CFLAGS}")
 GEM_THREADS_LIBS="${GEM_THREADS_LIBS}${GEM_THREADS_LIBS:+ }${PTHREAD_LIBS}"
fi
])
