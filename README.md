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

The folder containing the sources of Pd Vanilla and standard externals. This is a git submodule of Miller Puckette's [official Pd git repository](http://sourceforge.net/p/pure-data/pure-data/ci/master/tree):

    git://git.code.sf.net/p/pure-data/pure-data

If you're cloning this repo, make sure to checkout the submodule:

	git submodule init
	git submodule update

### libpd_wrapper

This folder contains the source files that make up the core of libpd.

### Android.mk, Makefile, libpd.xcodeproj, libpd_win.sln

Build support for various platforms. Feel free to improve the build system in any way you see fit.

Currently the main Makefile builds a dynamic lib on Windows (in MinGW), Linux, & Mac OSX and has the following targets:

  - **libpd**: (default) builds if no target is specified, builds the libpd.so/dylib/dll
  - **cpplib**: builds libpd with the cpp wrapper
  - **csharplib**: builds libpdcsharp.dll (tested on Windows only)
  - **javalib**: builds libpdnative and the jni wrapper
  - **clean**: removes the object files
  - **clobber**: removes the linked library files
  - **install**: installs libpd C library (+ C++ if built) and headers, set location with prefix= (default: /usr/local)
  - **uninstall**: removes libpd C library and headers, set location with prefix= (default: /usr/local)

Makefile options allow for conditional compilation of libpd util and pd extra externals sources into libpd as well as other options:

  - **UTIL=true**, compiles `libpd_wrapper/util` ringbuffer and print concatenator
  - **EXTRA=true**, compiles `pure-data/extra` externals which are then inited in libpd_init()
  - **DEBUG=true**, compiles libpd with -Wall & no optimizations

For example, to build libpd with both util and extra:

    make UTIL=true EXTRA=true

### java, csharp, objc, cpp, python

Glue for using libpd with Java, C#, Objective C, C++ and Python. Feel free to improve or add support for other languages such as Lua.

### samples

Contains small sample programs and tests in the various supported langauges.

Xcode Project
-------------

libpd.xcodeproj provides an Xcode project to build libpd + the Obj-C wrapper as a static library for iOS & Mac OSX. Drag the libpd project into your existing Xcode project, then add libpd-ios (or libpd-osx) to the Linked Frameworks and Libraries in the General tab of your project target.

If you are unfamiliar with how static libraries work or how to use them in Xcode, see [this useful tutorial](http://www.raywenderlich.com/41377/creating-a-static-library-in-ios-tutorial).

Java Builds
-----------

Ready-made binaries for Java are available at [libpd-java-build](https://github.com/wivlaro/libpd-java-build):
<https://github.com/wivlaro/libpd-java-build/blob/master/libpd.jar> (may not be up to date)

CocoaPods
---------

If you are using XCode to build iOS apps, you can use [CocoaPods](https://cocoapods.org) to add libpd to your project.

Use the following in your CocoaPods podfile:

    pod 'libpd', :git => 'https://github.com/libpd/libpd', :submodules => true

