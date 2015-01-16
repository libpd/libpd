For documentation of libpd, see the wiki: <https://github.com/libpd/libpd/wiki>

Layout of the libpd repository:

  * pure-data: The folder containing the sources of Pd Vanilla and standard
      externals. This is a git submodule of Miller Puckette's official Pd
      git repository, available at
      <git://pure-data.git.sourceforge.net/gitroot/pure-data/pure-data>  
	  
	  If you're cloning this repo, make sure to checkout the submodule:  
	  
	      git submodule init
		  git submodule update

  * libpd_wrapper: This folder contains the source files that make up the core
      of libpd.

  * Android.mk, Makefile, libpd.xcodeproj, libpd_win.sln: Build support for
      various platforms. Feel free to improve the build system in any way you see fit.
	  Currently the main Makefile builds a dynamic lib on Windows (in MinGW), Linux, &
	  Mac OSX and has the following targets: 
      - libpd, (default) builds if no target is specified, builds the libpd.so/dylib/dll
      - cpplib, builds libpd with the cpp wrapper
	  - csharplib, builds libpdcsharp.dll (tested on Windows only)
      - javalib, builds libpdnative and the jni wrapper
      - clean, removes the object files
      - clobber, removes the linked library files

  * java, csharp, objc, cpp, python: Glue for using libpd with Java, C#, Objective C,
      C++ and Python.  Feel free to improve or add support for other languages 
      such as Lua.

  * samples: This branch contains one small sample program in C built on top
      of libpd, as well as a sample project in Java that uses JavaSound.

Ready-made binaries for Java are available at
<https://github.com/airbaggins/libpd-java-build/raw/master/libpd.jar>
