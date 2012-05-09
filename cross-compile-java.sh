#!/bin/bash

make CC='gcc -m32' PDNATIVE_ARCH=x86    clean javalib
make CC='gcc -m64' PDNATIVE_ARCH=x86_64 clean javalib

mkdir -v -p java-build/org/puredata/natives/windows/{x86,x86_64}
pthread_w32_dll=`which pthreadGC2-w32.dll`
pthread_w64_dll=`which pthreadGC2-w64.dll`
if test $? -eq 0 ; then cp -v "$pthread_w32_dll" java-build/org/puredata/natives/windows/x86 ; fi
if test $? -eq 0 ; then cp -v "$pthread_w64_dll" java-build/org/puredata/natives/windows/x86_64 ; fi

WINDOWS_JAVA_HOME="/media/WINXP/Program Files/Java/jdk1.7.0_04"
if test -d "$WINDOWS_JAVA_HOME" ; then
  make OS=Windows_NT CC='i686-w64-mingw32-gcc -m32'   JAVA_HOME="$WINDOWS_JAVA_HOME" clean javalib
  make OS=Windows_NT CC='x86_64-w64-mingw32-gcc -m64' JAVA_HOME="$WINDOWS_JAVA_HOME" clean javalib
else
  echo >2 "Couldn't find JAVA_HOME=\"$JAVA_HOME\""
fi
