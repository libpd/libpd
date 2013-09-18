#-*- Mode: autoconf -*-
#
# SYNOPSIS
#
#   AX_DISTRIBUTE_CFLAGS([$FLAGS], [cppflagsVar], [cflagsVar])
#     + distributes $FLAGS among $cppflagsVar and $cflagsVar
#
#   AX_SAFE_CFLAGS([$FLAGS], [safeVar], [unsafeVar]).
#     + distributes $FLAGS among $safeVar and $unsafeVar
#
# LAST MODIFICATION
#
#   Sun, 26 Apr 2009 23:53:21 +0200
#
# COPYLEFT
#
#   Copyright (c) 2009 Bryan Jurish <moocow@ling.uni-potsdam.de>
#
#   Copying and distribution of this file, with or without
#   modification, are permitted in any medium without royalty provided
#   the copyright notice and this notice are preserved.


# AX_DISTRIBUTE_CFLAGS($FLAGS, CPPFLAGS, CFLAGS)
#  + distribute $FLAGS between $CPPFLAGS and $CFLAGS
AC_DEFUN([AX_DISTRIBUTE_CFLAGS],
[
 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## BEGIN AX_DISTRIBUTE_CFLAGS
 axdf_cppflags=
 axdf_cflags=

 for axdf_flag in $1
 do
   case "$axdf_flag" in
     -undef|-nostdinc*|-I*|-D*|-U*|-E|-P|-C)
       axdf_cppflags="$axdf_cppflags $axdf_flag"
       ;;
     *)
       axdf_cflags="$axdf_cflags $axdf_flag"
       ;;
   esac
 done

 test x$2 != x && $2="$$2 $axdf_cppflags"
 test x$3 != x && $3="$$3 $axdf_cflags"

 ## END AX_DISTRIBUTE_CFLAGS
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
])

# AX_SAFE_CFLAGS($FLAGS, SAFE, UNSAFE)
#  + distribute $FLAGS between $SAFE and $UNSAFE
AC_DEFUN([AX_SAFE_CFLAGS],
[
 ##vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 ## BEGIN AX_SAFE_CFLAGS
 axsf_safe=
 axsf_unsafe=

 for axsf_flag in $1
 do
   axsf_flag_tmp=`echo $axsf_flag | sed -e 's/[[ '\''\"\(\)]]//g'`
   if test "${axsf_flag_tmp}" = "${axsf_flag}"; then
     axsf_safe="$axsf_safe $axsf_flag"
   else
     axsf_unsafe="$axsf_unsafe $axsf_flag"
   fi
 done

 test x$2 != x && $2="$$2 $axsf_safe"
 test x$3 != x && $3="$$3 $axsf_unsafe"
 ## END AX_SAFE_CFLAGS
 ##^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
])
