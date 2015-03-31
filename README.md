libpd
=====

[Pure Data](http://puredata.info) as an embeddable audio synthesis library

Copyright (c) Peter Brinkmann & the libpd team 2010-2015

Documentation
-------------

See our website and book at <http://libpd.cc>

For documentation of libpd, see the wiki: <https://github.com/libpd/libpd/wiki>

If you are using [Processing](http://processing.org), iOS, or Android, see our companion repositories:

* [puredatap5](https://github.com/libpd/puredatap5)
* [pd-for-ios](https://github.com/libpd/pd-for-ios)
* [pd-for-android](https://github.com/libpd/pd-for-android)

Repository Layout
-----------------

### pure-data
  
The folder containing the sources of Pd Vanilla and standard externals. This is a git submodule of Miller Puckette's [official Pd git repository](http://sourceforge.net/p/pure-data/pure-data/ci/master/tree) (git://git.code.sf.net/p/pure-data/pure-data), available through a Github mirror:  
	  
    https://github.com/pure-data/pure-data.git  
	  
If you're cloning this repo, make sure to checkout the submodule:  
	  
	git submodule init
	git submodule update

### libpd_wrapper

This folder contains the source files that make up the core of libpd.

### Android.mk, Makefile, libpd.xcodeproj, libpd_win.sln

Build support for various platforms. Feel free to improve the build system in any way you see fit.
	  
Currently the main Makefile builds a dynamic lib on Windows (in MinGW), Linux, & Mac OSX and has the following targets: 	  
      
  - **libpd**, (default) builds if no target is specified, builds the libpd.so/dylib/dll
  - **cpplib**, builds libpd with the cpp wrapper
  - **csharplib**, builds libpdcsharp.dll (tested on Windows only)
  - **javalib**, builds libpdnative and the jni wrapper
  - **clean**, removes the object files
  - **clobber**, removes the linked library files
      
Makefile options allow for conditional compilation of libpd util and pd extra externals sources into libpd:

  - **UTIL=true**, compiles `libpd_wrapper/util` ringbuffer and print concatenator
  - **EXTRA=true**, compiles `pure-data/extra` externals which are then inited in libpd_init()
  
For example, to build libpd with both util and extra:

    make UTIL=true EXTRA=true

### java, csharp, objc, cpp, python

Glue for using libpd with Java, C#, Objective C, C++ and Python.  Feel free to improve or add support for other languages such as Lua.

### samples

Contains a small sample program in C built on top of libpd, a sample project in Java that uses JavaSound, and the cppTest & iOSTest Xcode projects.

Java Builds
-----------

Ready-made binaries for Java are available at [libpd-java-build](https://github.com/wivlaro/libpd-java-build): 
<https://github.com/wivlaro/libpd-java-build/blob/master/libpd.jar> (may not be up to date)

