dnl Copyright (C) 2005-2006 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

# AC_CHECK_CXXFLAGS(ADDITIONAL-CXXFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(CXX) compiler accepts the ADDITIONAL-CXXFLAGS
# if so, they are added to the CXXFLAGS
AC_DEFUN([AC_CHECK_CXXFLAGS],
[
  AC_MSG_CHECKING([whether $CXX accepts "$1"])
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
])# AC_CHECK_CXXFLAGS

# AC_CHECK_CFLAGS(ADDITIONAL-CFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(C) compiler accepts the ADDITIONAL-CFLAGS
# if so, they are added to the CFLAGS
AC_DEFUN([AC_CHECK_CFLAGS],
[
  AC_MSG_CHECKING([whether $CC accepts "$1"])
cat > conftest.c << EOF
int main(){
  return 0;
}
EOF
if $CC $CFLAGS [$1] -o conftest.o conftest.c > /dev/null 2>&1
then
  AC_MSG_RESULT([yes])
  CFLAGS="${CFLAGS} [$1]"
  [$2]
else
  AC_MSG_RESULT([no])
  [$3]
fi
])# AC_CHECK_CFLAGS

# AC_CHECK_FRAMEWORK(FRAMEWORK, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
#
AC_DEFUN([AC_CHECK_FRAMEWORK],
[
  AC_MSG_CHECKING([for "$1"-framework])

  temp_check_ldflags_org="${LDFLAGS}"
  LDFLAGS="-framework [$1] ${LDFLAGS}"

  AC_LINK_IFELSE(AC_LANG_PROGRAM(,), [temp_check_ldflags_success="yes"],[temp_check_ldflags_success="no"])

  if test "x$temp_check_ldflags_success" = "xyes"; then
    AC_MSG_RESULT([yes])
    [$2]
  else
    AC_MSG_RESULT([no])
    LDFLAGS="$temp_check_ldflags_org"
    [$3]
  fi
])# AC_CHECK_FRAMEWORK

# AC_CHECK_LDFLAGS(ADDITIONAL-LDFLAGS, ACTION-IF-FOUND, ACTION-IF-NOT-FOUND)
#
# checks whether the $(LD) linker accepts the ADDITIONAL-LDFLAGS
# if so, they are added to the LDFLAGS
AC_DEFUN([AC_CHECK_LDFLAGS],
[
  AC_MSG_CHECKING([whether linker accepts "$1"])
  temp_check_ldflags_org="${LDFLAGS}"
  LDFLAGS="$1 ${LDFLAGS}"

  AC_LINK_IFELSE(AC_LANG_PROGRAM(,), [temp_check_ldflags_success="yes"],[temp_check_ldflags_success="no"])

  if test "x$temp_check_ldflags_success" = "xyes"; then
    AC_MSG_RESULT([yes])
    [$2]
  else
    AC_MSG_RESULT([no])
    LDFLAGS="$temp_check_ldflags_org"
    [$3]
  fi
])# AC_CHECK_LDFLAGS

