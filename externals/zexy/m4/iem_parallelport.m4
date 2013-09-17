dnl Copyright (C) 2005-2006 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([IEM_CHECK_LPT],
[
dnl check for LPT
AC_ARG_ENABLE(lpt,    [  --enable-lpt            enable parallelport-support])

if test "x" = "x${enable_lpt}" ; then
 enable_lpt="${with_lpt}"
fi

AC_CHECK_HEADERS(linux/ppdev.h, [ have_ppdev=" (with device-support)" ], [ have_ppdev="" ])

if test x"$enable_lpt" != "xno"
then
  AC_MSG_CHECKING([parallel-port])
  if test "x$enable_lpt" = "xyes"
  then
# forced
    AC_DEFINE([Z_WANT_LPT], [1], [Define if you want parallelport support])
    have_lpt="yes (forced)"
  else
    AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/io.h>]], [[ ioperm(0x3bc, 8, 1); outb(0, 0x3bc); ioperm(0x3bc, 8, 0); ]])],
                    [have_lpt="yes"], [have_lpt="no"])
  fi
fi

if test "x$have_lpt" != "xno"; then
 AC_DEFINE([Z_WANT_LPT], [1], [Define if you want line printer support])
 AC_MSG_RESULT([$have_lpt$have_ppdev])
else
 AC_MSG_RESULT([no])
fi
AM_CONDITIONAL([LPT], [test "x${have_lpt}" != "xno"])

]) dnl IEM_CHECK_LPT
