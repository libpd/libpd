    README for pd external 'deque'

    Last updated for version 0.03.

DESCRIPTION
    Double-ended message-queue for pd.

PLATFORMS
    * linux/x86
        This is what I run, so things really ought to work here.

    * Other Platforms
        See REQUIREMENTS, below.

REQUIREMENTS
    In order to build the "deque" library, you will need the following:

    * A C compiler
        Tested with gcc-3.3.3 under linux/x86 (Debian).

    * /bin/sh , sed
        In order to run the 'configure' script.

    * A make program
        Tested with GNU make v3.79.1 under linux/x86 (Debian).

    * PD
        Tested with pd v0.37.1 under linux/x86 (Debian). PD is available
        from:

         http://www.crca.ucsd.edu/~msp/software.html

INSTALLATION
    Issue the following commands to the shell:

       cd PACKAGENAME-X.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    * --with-pd-dir=DIR
        PD base directory.

    * --with-pd-include=DIR
        Directory where the PD include files live.

    * --with-pd-extdir=DIR
        Where to put the externals on 'make install'.

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages and code. Default=no.

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and IOhannes m Zmoelnig.

KNOWN BUGS
    * General
        Only tested under linux.

AUTHOR / MAINTAINER
    Bryan Jurish <moocow@ling.uni-potsdam.de>

