dnl Copyright (C) 2005-2006 IOhannes m zmölnig
dnl This file is free software; IOhannes m zmölnig
dnl gives unlimited permission to copy and/or distribute it,
dnl with or without modifications, as long as this notice is preserved.

AC_DEFUN([IEM_CHECK_FAT],
[
AC_ARG_ENABLE(fat-binary,
       [  --enable-fat-binary=ARCHS
                          build an Apple Multi Architecture Binary (MAB);
                          ARCHS is a comma-delimited list of architectures for
                          which to build; if ARCHS is omitted, then the package
                          will be built for all architectures supported by the
                          platform (e.g. "ppc,i386" for MacOS/X and Darwin; 
                          if this option is disabled or omitted entirely, then
                          the package will be built only for the target 
                          platform],
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
   for archs in $TARGET_ARCHS 
   do
    []Name="$[]Name -arch $archs"
   done

   if test "x$[]Name" != "x"; then
    IEM_CHECK_CFLAGS($[]Name,,[]Name="")
    IEM_CHECK_CXXFLAGS($[]Name,,[]Name="")
    IEM_CHECK_LDFLAGS($[]Name,,[]Name="")
   fi


   undefine([Name])
fi
])# IEM_CHECK_FAT
