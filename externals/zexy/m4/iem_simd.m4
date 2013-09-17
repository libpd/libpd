dnl Copyright (C) 2005-2006 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([IEM_CHECK_SIMD],
[
AC_ARG_ENABLE(simd,
       [  --enable-simd=ARCHS
                          enable SIMD optimization;
                          valid arguments are: SSE2
       ],
       [simd=$enableval], [simd=no])
if test "$simd" != no; then
   AC_MSG_CHECKING([SIMD optimization])

   # Respect SIMD given to --enable-simd if present.
   if test "$simd" != yes; then
	    SIMD=`echo "$simd" | tr ',' ' '`
   else
	    # Choose a default set of architectures based upon platform.
      SIMD="SSE2"
   fi

   for smd in $SIMD 
   do
    case "${smd}" in
    SSE2|sse2)
      AC_MSG_RESULT([SSE2])
      IEM_CHECK_CFLAGS([-mfpmath=sse -msse])
      IEM_CHECK_CXXFLAGS([-mfpmath=sse -msse])
    ;;
    *)
      AC_MSG_RESULT([unknown SIMD instructions: ${smd}])
    ;;
    esac
   done
fi
])# IEM_CHECK_SIMD
