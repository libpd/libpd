For documentation of libpd, see the wiki: http://gitorious.org/pdlib/pages/Home

Layout of the libpd repository:

  * pure-data: The folder containing the sources of Pd Vanilla and standard
      externals.  This subtree is a subset of Miller Puckette's official Pd
      git repository, available at
      git://pure-data.git.sourceforge.net/gitroot/pure-data/pure-data,
      with minimal changes.

      We're keeping our own copy of the sources of Pd in order to have the
      option of adding optimizations and other changes that may not make it
      into the official version immediately, but the overall policy is to
      maintain compatibility with the official version.  Please don't make any
      incompatible changes in the pure-data branch, and don't add anything
      specific to libpd that you wouldn't expect to eventually be merged into
      the official version.

  * libpd_wrapper: This folder contains the source files that make up the core
      of libpd.

  * Android.mk, Makefile, libpd.xcodeproj: Build support for
      various platforms.  I figure the Android makefile is pretty much
      finished, but the build system for other platforms still needs work.
      The Makefile for build shared libraries on Linux is crude but
      functional.  The one for Macs is tentative and untested, and we
      currently don't have a makefile for Windows.  Feel free to improve the
      build system in any way you see fit.

  * java, objc, python: Glue for using libpd with Java, Objective C, and
      Python.  Feel free to improve or add support for other languages such as
      Lua.

  * samples: Currently, this branch only contains one small sample program in
      C built on top of libpd.  It's probably redundant, but it may give you
      an idea how to embed libpd into your own applications.

