    README for example pd external package 'hello'

    Last updated for hello v0.01

DESCRIPTION
    The 'hello' package is an example external package intended to
    demonstrate one way of using GNU autotools (automake, autoconf,
    autoheader) to manage a Pd external distribution.

    See "HOWTO" for a developer-oriented introduction.

INSTALLATION
    Issue the following commands to the shell:

       cd PACKAGE-XX.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

CONFIGURATION OPTIONS
    The 'configure' script supports the following options, among others:

    * --help
        Output a brief usage summary of the 'configure' script, including a
        list of supported options and influential environment variables.

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

    * --with-pd-dir=PD_DIR
        Set base Pd directory. Default PREFIX/pd

    * --with-pd-include=PD_INC
        Where to look for m_pd.h

    * --with-pd-extdir=PD_EXTERNS
        Where to install compiled externals. Default: PD_DIR/externs

    * --enable-object-externals , --disable-object-externals
        Whether to build single-object externals or only multilibs
        (default), for packages which support both.

HOWTO
    This section provides a brief developer-oriented description of how to
    use GNU autotools to manage your own Pd external package.

  Files
    $(top_srcdir)/common
        The directory common/ includes common code for the autotools
        packages. Copy it to your top-level package directory.

    $(top_srcdir)/common/m4
        Contains m4 macros for autotools. See "ax_pd_external.m4" for
        details.

    $(top_srcdir)/common/pdexternal.am
        May be included in your package's Makefile.am. See "pdexternal.am"
        for details.

    $(top_srcdir)/common/mooPdUtils.h (optional)
        The file mooPdUtils.h may be included by your C source files.
        Currently, this only provides a PDEXT_UNUSED macro to avoid annoying
        gcc warnings under -W.

  Running aclocal
    You must pass the "-I common/m4" flag to aclocal when you call it. For
    maintainer-mode rebuilding and autoreconf, you should add the following
    to your top-level Makefile.am:

     ACLOCAL_AMFLAGS = -I $(top_srcdir)/common/m4

    See the example package's autogen.sh for a useful wrapper script.

  configure.ac
    You must call the macro AX_PD_EXTERNAL from configure.ac. Before doing
    so (and before calling AC_PROG_CC), make sure that you cache the values
    of important user flags in shell variables:

     ##-- save user's CFLAGS,CPPFLAGS
     UCPPFLAGS="$CPPFLAGS"
     UCFLAGS="$CFLAGS"
     ULDFLAGS="$LDFLAGS"

     ##-- Pd external stuff
     AX_PD_EXTERNAL

    See the example package's configure.ac for a complete working example.

  Makefile.am
    You probably want to include $(top_srcdir)/common/pdexternal.am in your
    Makefile.am(s). This will allow you to build Pd externals as "_PROGRAMS"
    targets. In particular, pdext_PROGRAMS targets will be built as
    externals and installed in PDEXT_DIR (see above).

    See the example package's Makefile.am for a complete working example.

    Externals
        To build & install the external "hello.$(PDEXT)", add the following
        to Makefile.am:

         pdexterns_PROGRAMS = hello
         hello_SOURCES = hello.c mooPdUtils.h

    Abstractions
        To install the abstraction "heynow.pd", add the following to
        Makefile.am:

         pdexterns_DATA = heynow.pd

    Documentation
        To install the documentation patch "hello-help.pd", add the
        following to Makefile.am:

         pddoc_DATA = hello-help.pd

  ax_pd_external.m4
    The AX_PD_EXTERNAL macro defined in common/m4/ax_pd_external.m4 is
    intended to perform all common autoconf-level checks and substitutions
    necessary for building Pd external packages on various systems. Among
    other things, this includes:

    *   Providing --with-FEATURE and --enable-FEATURE arguments such as
        --with-pd-dir (see "CONFIGURATION OPTIONS", above).

    *   Defining automake-style "Xdir" variables for easy definition of
        package externals, abstractions, and documentation.

    *   Checking for the required header "m_pd.h".

    *   Defining platform-dependent compiler and linker flags for building
        Pd externals, and adding these to the relevant system variables
        (CPPFLAGS, CFLAGS, LDFLAGS, etc.)

    See the comments at the top of m4/ax_pd_external.m4 for more details on
    the AX_PD_EXTERNAL macro.

  pdexternal.am
    pdexternal.am is intended to be included in your package's Makefile.am.
    It redefines the automake EXEEXT to allow building Pd externals using
    automake's _PROGRAMS targets. Additionally, it defines the automake
    varibles PDEXT, SUFFIXES, EXTRA_DIST, CLEANFILES, DISTCLEANFILES, and
    MAINTAINERCLEANFILES.

  Multilibs and Single-Object Externals
    You can use automake's EXTRA_PROGRAMS variable, together with the
    pdexterns_PROGRAMS automake target, the automake conditional
    WANT_OBJECT_EXTERNALS, and the C preprocessor macro
    WANT_OBJECT_EXTERNALS to allow building single-object-externals or
    multilibs from a single source distribution.

    Makefile.am should contain something like:

     ##-- always build these externals
     pdexterns_PROGRAMS = hello

     ##-- potential single-object externals (as _PROGRAMS)
     EXTRA_PROGRAMS = goodbye

     ##-- build single-object externals?
     if WANT_OBJECT_EXTERNALS
      pdexterns_PROGRAMS += goodbye
     endif

    If single-object externals were requested by the user, then the C
    preprocessor macro WANT_OBJECT_EXTERNALS will be defined by autoheader,
    to allow you to conditionally #include<> the EXTRA_PROGRAMS sources in
    your top-level multilib source file if desired. In the above example,
    hello.c might contain:

     #ifdef HAVE_CONFIG_H
     # include "config.h"
     #endif

     #ifndef WANT_OBJECT_EXTERNALS
     /*-- Multilib build: include source for the goodbye external --*/
     # include "goodbye.c"
     #endif

     /*... local definitions etc. go here ...*/

     void hello_setup(void)
     {
      #ifndef WANT_OBJECT_EXTERNALS
       goodbye_setup();
      #endif

      /*... local setup code goes here ...*/
     }

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

