libpd
=====

[Pure Data](http://puredata.info) as an embeddable audio synthesis library

Copyright (c) Peter Brinkmann & the libpd team 2010-2016

Documentation
-------------

See our website and book at <http://libpd.cc>

For documentation of libpd, see the wiki: <https://github.com/libpd/libpd/wiki>

If you are using [Processing](http://processing.org), iOS, or Android, see our companion repositories:

* [puredatap5](https://github.com/libpd/puredatap5)
* [pd-for-ios](https://github.com/libpd/pd-for-ios)
* [pd-for-android](https://github.com/libpd/pd-for-android)

Getting libpd
-------------

The preferred method to download libpd is to use git.

**Do not download libpd as a zip or tar.gz file from GitHub.**

The "Download zip" button may look like a good idea, but currently Github does not include submodule files when compiling zip files. This means the zip file is missing the main pd source files and you will not be able to build libpd, with errors such as: *No rule to make target `pure-data/src/d_arithmetic.o'* or *No such file or directory: pure-data/extra/bonk~/bonk~.c*.

To download libpd & checkout the pure-data submodule do the following:

    git clone https://github.com/libpd/libpd.git
    cd libpd
    git submodule init
    git submodule update
    
You should now have a `libpd` directory and the `libpd/pure-data` should contain the pd sources. If not, make sure you ran the git submodule commands in the libpd folder itself.

For most uses, it is recommended to checkout the latest stable release version via a git tag. For example, to switch to libpd version 0.8.3 after cloning:

    git checkout 0.8.3

The master branch contains the latest libpd development and can be considered *generally* stable. However, we make no guarantees. :)

Repository Layout
-----------------

### pure-data

The folder containing the sources of Pd Vanilla and standard externals. This is a git submodule of Miller Puckette's [official Pd git repository](http://sourceforge.net/p/pure-data/pure-data/ci/master/tree):

    git://git.code.sf.net/p/pure-data/pure-data

### libpd_wrapper

This folder contains the source files that make up the core of libpd.

### Android.mk, Makefile, libpd.xcodeproj, libpd_csharp.sln

Build support for various platforms. Feel free to improve the build system in any way you see fit.

Currently the main Makefile builds a dynamic lib on Windows (in MinGW), Linux, & Mac OSX and has the following targets:

  - **libpd**: (default) builds if no target is specified, builds the libpd.so/dylib/dll
  - **cpplib**: builds libpd with the cpp wrapper
  - **csharplib**: builds libpdcsharp.dll (on Windows) or libpdcsharp.so (on Linux)
  - **javalib**: builds libpdnative and the jni wrapper
  - **clean**: removes the object files
  - **clobber**: removes the linked library files
  - **install**: installs libpd C library (& C++ if built) and headers, set location with prefix= (default: /usr/local)
  - **uninstall**: removes libpd C library and headers, set location with prefix= (default: /usr/local)

Makefile options allow for conditional compilation of libpd util and pd extra externals sources into libpd as well as other options:

  - **UTIL=true**, compiles `libpd_wrapper/util` ringbuffer and print concatenator
  - **EXTRA=true**, compiles `pure-data/extra` externals which are then inited in libpd_init()
  - **DEBUG=true**, compiles libpd with -Wall & no optimizations
  - **LOCALE=true**, set the number format to the default "C" locale\*

For example, to build libpd with both util and extra:

    make UTIL=true EXTRA=true

_Note: cpplib is automatically built with UTIL=true as it uses the ringbuffer_

\* Setting the number locale explictly may be necessary if you run into certain numbers not being read correctly when opening patches. See <https://github.com/libpd/libpd/issues/130> for more info.

### java, csharp, objc, cpp, python

Glue for using libpd with Java, C#, Objective C, C++ and Python. Feel free to improve or add support for other languages such as Lua.

### samples

Contains small sample programs and tests in the various supported langauges.

Xcode Project
-------------

libpd.xcodeproj provides an Xcode project to build libpd + the Obj-C wrapper as a static library for iOS & Mac OSX. Drag the libpd project into your existing Xcode project, then add libpd-ios (or libpd-osx) to the Linked Frameworks and Libraries in the General tab of your project target.

For detailed instructions, see [Working with libpd in Xcode](libpd/libpd/wiki/Working-with-libpd-in-Xcode)

If you are unfamiliar with how static libraries work or how to use them in Xcode, see [this useful tutorial](http://www.raywenderlich.com/41377/creating-a-static-library-in-ios-tutorial).

_Note: libpd is tested with the release versions of Xcode. It is recommended that you avoid using beta or developer preview versions._

Java Builds
-----------

Ready-made binaries for Java are available at [libpd-java-build](https://github.com/wivlaro/libpd-java-build):
<https://github.com/wivlaro/libpd-java-build/blob/master/libpd.jar> (may not be up to date)

CocoaPods
---------

If you are using Xcode to build iOS apps, you can use [CocoaPods](https://cocoapods.org) to add libpd to your project.

Use the following in your CocoaPods podfile:

    pod 'libpd', :git => 'https://github.com/libpd/libpd', :submodules => true

C#
--

### Installation from NuGet
LibPD is available as a [NuGet package](https://www.nuget.org/packages/LibPdBinding). If your platform's native dll is not included, you have to build it yourself with `make csharplib` and copy the resulting file to the output directory. Batch scripts for compilations on Windows with MinGW64 are included.

### Building yourself
The C# library expects a file libpdcsharp.dll in its folder. Before using the project, you need to compile it.

Include `csharp/LibPdBinding.csproj` in your solution and reference the project in your application.

#### Windows
The wrapper can be built with [MinGW-w64](http://mingw-w64.org/doku.php).

You need to install [msys2](http://msys2.github.io/), preferably the version for i686, because that version can build the 64bit versions as well.

Install msys2 using the instructions on the download page.

Install make and gcc by running `pacman -S msys/make mingw32/mingw-w64-i686-gcc mingw64/mingw-w64-x86_64-gcc` from the msys2 console. You can search for packages in msys2 with `pacman -S -s <searchterm>`.

For the 64 bit version of msys2 you also need to install winpthread by running `pacman -S msys/mingw-w64-cross-winpthreads-git`.

Edit `mingw32_build_csharp.bat` or `mingw64_build_csharp.bat` and execute it to create the native dll. You probably only need to change the variable for `%MSYS2%`. Usually you want the 32 bit version, as it will work on 64 bit Windows as well, but Unity 5 needs the 64 bit version.

For the 64 bit version, you also must use `libs/mingw64/libwinptread-1.dll` instead of `libs/mingw32/libwinpthread-1.dll`.

For a current version of `libwinpthread-1.dll` search in your msys2 installation folders.

#### Linux 
If you want to use the library on Linux with Mono, you need the following changes to the LibPdBinding project:

  - Compile the so file with `make csharplib`.
  - Remove `libpdcsharp.dll` and `libwinpthread-1.dll` from LibPdBinding project.
  - Add `libpdcsharp.so` to the LibPdBinding project.
  - Set "Copy to Output Directory" for `libpdcsharp.so` to "Copy always"
