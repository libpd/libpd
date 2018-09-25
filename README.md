libpd
=====

[Pure Data](http://puredata.info) as an embeddable audio synthesis library

Copyright (c) Peter Brinkmann & the libpd team 2010-2018

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

The "Download zip" button may look like a good idea, but currently Github does not include submodule files when compiling zip files. This means the zip file is missing the main pd source files and you will not be able to build libpd, with errors such as: *No rule to make target `pure-data/src/d_arithmetic.o`* or *No such file or directory: pure-data/extra/bonk~/bonk~.c*.

To download libpd & checkout the pure-data submodule do the following:

    git clone https://github.com/libpd/libpd.git
    cd libpd
    git submodule init
    git submodule update
    
You should now have a `libpd` directory and the `libpd/pure-data` should contain the pd sources. If not, make sure you ran the git submodule commands in the libpd directory itself.

For most uses, it is recommended to checkout the latest stable release version via a git tag. For example, to switch to libpd version 0.8.3 after cloning:

    git checkout 0.8.3

The master branch contains the latest libpd development and can be considered *generally* stable. However, we make no guarantees. :)

Repository Layout
-----------------

### pure-data

The directory containing the sources of Pd Vanilla and it's standard externals. This is a git submodule of Miller Puckette's [official Pd git repository](http://sourceforge.net/p/pure-data/pure-data/ci/master/tree):

    git://git.code.sf.net/p/pure-data/pure-data

### libpd_wrapper

This directory contains the source files that make up the core of libpd.

### Android.mk, Makefile, libpd.xcodeproj, libpd_csharp.sln, .classpath, .project

Build support for various platforms. Feel free to improve the build system in any way you see fit.

### cpp, csharp, java, jni, objc, python

Glue for using libpd with C++, C#, Java, Objective-C, and Python. Feel free to improve or add support for other languages such as Lua.

### samples

Small sample programs and tests in the various supported languages.

### libs

The build result location and required software libraries for the various supported languages.

Building libpd
--------------

Core build requirments:

* Unix command shell: bash, dash, etc
* C compiler chain: gcc/clang & make

Note: The various language wrappers may have additional requirements.

Currently the main Makefile builds a dynamic lib on Windows (in MinGW), Linux, & Mac OSX and has the following targets:

* **libpd**: build the libpd C core, default if no target is specified
* **csharplib**: build libpdcsharp
* **javalib**: build libpdnative and the jni wrapper
* **javadoc**: generate Java HTML documentation
* **javasrc**: create a Java source jar
* **clean**: remove object files
* **clobber**: remove linked library files
* **install**: install libpd C library and C/C++\* headers, set location with prefix= (default: /usr/local)
* **uninstall**: remove libpd C library and C/C++ headers, set location with prefix= (default: /usr/local)

\* _C++ headers are only installed if the C utility layers were built as well (ie. UTIL=true), see below._

Makefile options allow for conditional compilation of libpd util and pd extra externals sources into libpd as well as other options:

* **UTIL=true**: compile utilities in `libpd_wrapper/util` (default)
* **EXTRA=true**: compile `pure-data/extra` externals which are then inited in libpd_init() (default)
* **MULTI=true**: compile with multiple instance support
* **DEBUG=true**: compile with debug symbols & no optimizations
* **LOCALE=false**: do not set the LC_NUMERIC number format to the default "C" locale\* (default)
* **PORTAUDIO=true**: compile with portaudio support (currently JAVA jni only)
* **JAVA_HOME=/path/to/jdk**: specify the path to the Java Development Kit

To build the libpd C core with default options:

    make

To build libpd without the util libs and extra externals:

    make UTIL=false EXTRA=false

_Note: The C++ wrapper requires UTIL=true as it uses the ringbuffer._

\* See the Known Issues section for more info.

If you need to add custom search paths to the CFLAGS or LDFLAGS, you can specify them when building via the ADDITIONAL_* variables:

    make ADDITIONAL_CFLAGS="-I/usr/local/include" \
         ADDITIONAL_LDFLAGS="-L/usr/local/lib"

Once libpd has built successfully, the compiled library will be found in the `libs` directory.

### Linux & BSD

Install the core build requirements using your distribution's package manager. For Debian, you can install the compiler chain, autotools, & gettext with:

    sudo apt-get install build-essentials

### macOS

macOS is built on top of a BSD system and the bash commandline can be accessed with the Terminal application in the /Applications/Utility directory.

The clang compiler and associated tools are provided by Apple. If you are running macOS 10.9+, you *do not* need to install the full Xcode application and can install the Commandline Tools Package only by running the following:

    xcode-select --install

If you are running macOS 10.6 - 10.8, you will need to install Xcode from the Mac App Store or downloaded from <http://developer.apple.com>

### Windows

libpd on Windows can be built with either MinGW or Cygwin which provide the core build requirements: a compiler chain & shell environment.

It is recommended to use the Msys2 distribution which provides both a Unix command shell and MinGW. Download the Msys2 "x86_64" 64 bit installer (or "i686" if you are using 32 bit Windows) from:

    http://www.msys2.org/

Then install to the default location (C:\msys32 or C:\msys64) and follow the setup/update info on the Msys2 webpage.

Msys2 provides both 32 and 64 bit MinGW and command shells which are used to compile for 32 or 64 bit, respectively. Due to how MinGW is designed, you cannot build a 64 bit libpd with a 32 bit MinGW and vice versa.

Note: Msys2 development seems to change frequently, so some of the package names
      below may have changed after this document was written.

Open an Msys2 shell and install the compiler chain, autotools, & gettext via:

    # 32 bit
    pacman -S mingw-w64-i686-toolchain mingw-w64-i686-clang make
    
    # 64 bit
    pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-clang make

_You can also search for packages in Msys2 with `pacman -S -s <searchterm>`._

Once the packages are installed, you should now be ready to build libpd.

C++
---

The C++ wrapper is inspired by the Java wrapper and provides a PdBase class as well as listener, list, and message type classes. This is a header only library so you only need to include the `cpp` directory in your project. You also may need to add `libpd_wrapper/util` to you include paths.

Sample programs are found in `samples/cpp`.

C\#
--

### Installation from NuGet

The libpd C\# library is available as a NuGet package:

<https://www.nuget.org/packages/LibPdBinding>

If your platform's native libpdcsharp.(so/dll) is not included, you have to build it yourself following and copy the resulting file to the output directory. Batch scripts for compilation on Windows with MinGW are included.

### Building yourself

The C\# library expects a file libpdcsharp.(so/dll) in its directory. Before using the project, you need to compile it:

    make csharplib

Include `csharp/LibPdBinding.csproj` in your solution and reference the project in your application. See `csharp/README.txt` for details.

#### Windows

The wrapper can be built with MinGW. See the previous "Windows" section for instructions on setting up a MinGW-based build environment using Msys2.

Build libpdcsharp using the the .bat DOS batch file wrappers for make to match which MinGW you are using:

    # 32 bit
    ./mingw32_build_csharp.bat
    
    # 64 bit
    ./mingw64_build_csharp.bat

Usually you want the 32 bit version, as it will work on 64 bit Windows as well. However so C\# environments require a 64 bit version, Unity 5 for instance.

Once the build is finished, a libpdcsharp.(so/dll) library should be found in the `libs` directory

You also may need to use the libwinpthread library along with libpdcsharp. This included with libpd in the `libs` directory, either within `libs/mingw32` or `libs/mingw64`. For a current version of `libwinpthread-1.dll` search in your Msys2 installation's `bin` directory.

_Note: If you have installed Msys2 to a non-default location, you will need to change the variable for `%MSYS2%` in the .bat files._

#### Linux

If you want to use the library on Linux with Mono, you need the following changes to the LibPdBinding project:

1. Compile the .so file with `make csharplib`.
2. Remove `libpdcsharp.dll` and `libwinpthread-1.dll` from LibPdBinding project.
3. Add `libpdcsharp.so` to the LibPdBinding project.
4. Set "Copy to Output Directory" for `libpdcsharp.so` to "Copy always"

Java
----

### Precompiled Binaries

**May be out of date**

Ready-made binaries for Java are available at the libpd-java-build repository:

<https://github.com/wivlaro/libpd-java-build>

### Building Yourself

You will need the Java Development Kit (JDK) to build the libpd Java lib. Make sure the JDK/bin path is added to your $PATH shell variable and, optionally, the JAVA_HOME variable points to the JDK location.

Build the libpd Java lib with:

    make javalib

This should result in a libpd.jar and pdnative.(so/dll) in the `libs` directory.

Optionally, you can build libpd with Eclipse using the `.classpath` & `.project` workspace files.

You can also build a libpd source jar and Java HTML documentation:

    make javasrc
    make javadoc

This should result in a `libs/libpd-sources.jar` and a `javadoc` directory.

#### Linux & BSD

Install the JDK via your distributions package manager.

### macOS

Install the JDK either by downloading an installer package or by using one of the open source package managers for macOS:

* homebrew: <https://brew.sh> (recommended)
* macports: <https://www.macports.org>

### Windows

The wrapper can be built with MinGW. See the previous "Windows" section for instructions on setting up a MinGW-based build environment using Msys2.

Install the JDK by downloading an installer package, then add the path to JDK/bin to your $PATH shell variable and the JDK path to $JAVA_HOME (optional). If the JDK is installed to `C:\Program Files\Java\jdk1.8.0_152`, add the following to your ~/.bash_profile:

    # add JDK bin path
    export PATH=$PATH:'C:\Program Files\Java\jdk1.8.0_152\bin'
    
    # JDK path (optional)
    export JAVA_HOME=C:/Program\ Files/Java/jdk1.8.0_152

Restart your shell if it's open.

Build the libpd javalib with:

    make javalib

You can also set the JAVA_HOME path when running make with:

    make javalib JAVA_HOME=C:/Program\ Files/Java/jdk1.8.0_152

Once the build is finished, you should find libpd.jar and pdnative.(so/dll) in the `libs` directory.

Objective-C
-----------

The Objective-C wrapper is designed to be used on iOS and macOS and includes a (currently iOS-only) audio unit and audio manager for sound I/O.

#### Xcode Project

libpd.xcodeproj provides an Xcode project to build libpd + the Obj-C wrapper as a static library for iOS & macOS. Drag the libpd project into your existing Xcode project, then add libpd-ios (or libpd-osx) to the Linked Frameworks and Libraries in the General tab of your project target.

The Xcode project builds the following targets:

* **libpd-ios**: libpd and the Obj-C wrapper for iOS
* **libpd-osx**: libpd and the Obj-C wrapper for macOS
* **libpd-ios-multi**: libpd for iOS with multiple instance support
* **libpd-osx-multi**: libpd for macOS with multiple instance support

For detailed instructions, see [Working with libpd in Xcode](libpd/libpd/wiki/Working-with-libpd-in-Xcode)

If you are unfamiliar with how static libraries work or how to use them in Xcode, see [this useful tutorial](http://www.raywenderlich.com/41377/creating-a-static-library-in-ios-tutorial).

_Note: libpd is tested with the release versions of Xcode. It is recommended that you avoid using beta or developer preview versions._

### CocoaPods

If you are using Xcode to build iOS apps, you can use [CocoaPods](https://cocoapods.org)to add libpd to your project.

Use the following in your CocoaPods podfile:

    pod 'libpd', :git => 'https://github.com/libpd/libpd', :submodules => true

Python
------

The Python wrapper provides a "pylibpd" module mirroring the libpd C API. Build the wrapper with:

    cd python
    make

See the sample programs in `samples/python`. Note, some samples require the "pyaudio" Portaudio library.

Building with CMake
-------------------

CMake can be used to build the libpd C library.

CMake is a cross-platform, open-source build system. CMake is used to control the software compilation process using simple platform and compiler independent configuration files, and generate native makefiles and workspaces that can be used in the compiler environment of your choice.

### Dependencies

* CMake: You can download CMake for your platform [here](https://cmake.org). The
minimum version is 2.8.11.
* pthreads: On Windows, you need to provide a pthreads library.

If you are using MinGW, you can use the libwinpthread-1.dll included in the `libs/mingw*` directories in this repository. Alternatively, you can also download it or compile it yourself from the sources [here](https://www.sourceware.org/pthreads-win32/). This will typically result in pthreadGC2.(dll/lib).

If you are using Visual Studio, you need to provide a pthreads library compiled for Visual Studio either by downloading it or compiling it yourself. See [here](https://www.sourceware.org/pthreads-win32/). Be careful to download / compile the right version for your setup. This would typically be pthreadVC2.(dll/lib).

### Configuring the build

One way to configure CMake is to use the [CMake GUI](https://cmake.org/runningcmake/). The GUI will list the variables that can be provided to configure the build. The variables can also be specified in the command-line interface (See below for an example)

In this step you can select the features to be included with `PD_EXTRA`, `PD_LOCALE`, `PD_MULTI` and `PD_UTILS` as described above. You can also enable building the C sample programs using `PD_BUILD_C_EXAMPLES`.

When using Microsoft Visual Studio (MSVC), you will be requested to provide a path to the pthreads library and its headers using variables `CMAKE_THREAD_LIBS_INIT` and `PTHREADS_INCLUDE_DIR`.

On macOS, you can define different deployment target and architectures from your current system using the variables `CMAKE_OSX_DEPLOYMENT_TARGET` and `CMAKE_OSX_ARCHITECTURES`.

You can specify additional compilation flags using the variable `CMAKE_C_FLAGS`.

CMake can now generate Makefiles, a MSVC solution, or an XCode project.

### Building

After generation, depending on your platform you can navigate to the directory where
CMake generated the build files and then:

* On Linux: run `make`
* On Windows: open the MSVC solution and build it
* On macOS: open the XCode project and build it

Of course you can also use CMake itself to build libpd by running this on the command line:

    cd <path/to/build/files/generated/by/CMake>
    cmake --build .
    
### Windows cross-compilation on Linux

You can build Windows binaries in Linux using MinGW. This will only build the static and shared libraries, and not the C\# bindings.

The easiest way is to set up a toolchain file for CMake pointing to all the necessary MinGW tools. For debian-based distributions, the file would contain the following:

    SET(CMAKE_SYSTEM_NAME Windows)

    SET(CMAKE_C_COMPILER x86_64-w64-mingw32-gcc)
    SET(CMAKE_CXX_COMPILER x86_64-w64-mingw32-g++)
    SET(CMAKE_RC_COMPILER x86_64-w64-mingw32-windres)

    SET(CMAKE_FIND_ROOT_PATH /usr/x86_64-w64-mingw32)

    SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
    SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
    SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

Note paths and binary names might vary in other distributions.

You can then use the [`CMAKE_TOOLCHAIN_FILE`](https://cmake.org/cmake/help/v3.12/variable/CMAKE_TOOLCHAIN_FILE.html) option to tell CMake to load the settings from your file. For example, if the above is saved to `$HOME/.local/share/cmake/toolchain-mingw-x86_64-w64.cmake`, then CMake can be invoked with 

    cmake -DCMAKE_TOOLCHAIN_FILE=$HOME/.local/share/cmake/toolchain-mingw-x86_64-w64.cmake
    
followed by any other CMake options you'd normally use, and the source directory.
    
### Command-line examples

Here are examples of how to download, configure and build the latest libpd on the command line using CMake.

Linux:

    git clone https://github.com/libpd/libpd
    cd libpd
    git submodule init
    git submodule update
    mkdir build && cd build
    cmake .. -DPD_MULTI:BOOL=ON -DPD_BUILD_C_EXAMPLES:BOOL=ON
    cmake --build .

Windows / MSVC:

    git clone https://github.com/libpd/libpd
    cd libpd
    git submodule init
    git submodule update
    mkdir build && cd build
    cmake .. -DCMAKE_THREAD_LIBS_INIT:PATH=</path/to/pthreadsVC2.lib> -DPTHREADS_INCLUDE_DIR:PATH=</path/to/pthread/header/files>
    cmake --build .

### Limitations

Currently the CMake script is not capable of building the C# or the Java bindings. Please use the makefile for that.      

Known Issues
------------

### How do I use libpd in Visual Studio?

Historically, Pd was designed to be built using the open source gcc & make and did not directly support being built in Visual Studio on Windows, mainly due to differences in C compiler versions.

Recently, the code has been adapted and a CMake build script has been developed that should allow you to generate a MSVC solution. The only important thing you need to be careful about is providing a pthreads library compiled for Visual Studio. See the section above about building with CMake.

Another possible approach is building the libpd C library using gcc and make using MinGW in msys on Windows. You can use the resulting .dll, .def, & .lib files with Visual Studio and the cpp wrapper is provided as an all header library so it should work directly within VS as well.

After building libpd in msys, you can "install" it to a temp directory to get only the libs and headers you need:

    make install prefix=libpd-build

### Problems with numbers in loaded patches or DSP output always seems to be 0

Pd expects numbers to be in an English format, ie. "0.3". If you are using a non-English language or locale setting on your system, it may be encoding numbers differently, ie. "0,3". This can lead to weird bugs in loaded patches where numbers seem wrong or end up truncated as 0.

By default, libpd is built with the LC\_NUMERIC locale set to the "C" default, so this shouldn't be a problem. If you are using libpd within a project that requires specific locale settings, you will need to make sure libpd's LC\_NUMERIC is left alone or at least reset it to "C" if working with a different numeric setting. If a non-English LC\_NUMERIC is set, you will run into the number parsing issues mentioned above.

If you need to control LC\_NUMERIC manually, you can build libpd without the call to setlocale() in libpd_init using the SETLOCALE=false makefile option or by setting the LIBPD_NO_NUMERIC define.

See <https://github.com/libpd/libpd/issues/130> for more info.
