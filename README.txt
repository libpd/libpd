For documentation of libpd, see the wiki: https://github.com/libpd/libpd/wiki

Layout of the libpd repository:

  * pure-data: The folder containing the sources of Pd Vanilla and standard
      externals.  This subtree is a subset of Miller Puckette's official Pd
      git repository, available at
      git://pure-data.git.sourceforge.net/gitroot/pure-data/pure-data.

      We're keeping our own copy of the sources of Pd in order to have the
      option of adding optimizations and other changes that may not make it
      into the official version immediately, but the overall policy is to
      maintain compatibility with the official version.  Please don't make any
      incompatible changes in the pure-data branch, and don't add anything
      specific to libpd that you wouldn't expect to eventually be merged into
      the official version.

  * libpd_wrapper: This folder contains the source files that make up the core
      of libpd.

  * Android.mk, Makefile, libpd.xcodeproj, libpd_win.sln: Build support for
      various platforms.  I figure the Android makefile is pretty much
      finished, but the build system for other platforms still needs work.
      The Makefile for build shared libraries on Linux is crude but
      functional.  The one for Macs is tentative and untested, and we
      currently don't have a makefile for Windows.  Feel free to improve the
      build system in any way you see fit. Currently the makefile has the 
      following targets: 
      - libpd, (default) builds if no target is specified, builds the libpd.so/dylib/dll
      - csharplib, builds libpdcsharp.dll
      - javalib, builds libpdnative and the jni wrapper
      - clean, removes the object files
      - clobber, removes the linked library files

  * java, csharp, objc, cpp, python: Glue for using libpd with Java, C#, Objective C,
      C++ and Python.  Feel free to improve or add support for other languages 
      such as Lua.

  * samples: This branch contains one small sample program in C built on top
      of libpd, as well as a sample project in Java that uses JavaSound.

Ready-made binaries for Java are available at
https://github.com/airbaggins/libpd-java-build/raw/master/libpd.jar.
