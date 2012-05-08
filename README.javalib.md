## Quickstart

Hopefully, you should just be able to include libs/libpd.jar in your project and
you're good to go. Otherwise, you can try and run:

    $ make clean javalib

Which should try and build a binary for your platform and add it to the libpd.jar.

## Cross-compilation

### Cross-compiling 32-bit x86 Linux from x86_64 Linux:

    $ make CC='gcc -m32' PDNATIVE_ARCH=x86 clean javalib

### Cross-compiling for Windows from Linux:

Using mingw-w64, based on instructions here http://tinyurl.com/mw64-use
Also, install the mingw-w64 pthreads, as http://tinyurl.com/mw64-pthread

#### Cross-compiling for 32-bit Windows from Linux:

    $ make OS=Windows_NT CC='i686-w64-mingw32-gcc -m32' JAVA_HOME="/media/WINXP/Program Files/Java/jdk1.7.0_04" clean javalib

#### Cross-compiling for 64-bit Windows from Linux:

    $ make OS=Windows_NT CC='x86_64-w64-mingw32-gcc -m64' JAVA_HOME="/media/WINXP/Program Files/Java/jdk1.7.0_04" clean javalib

