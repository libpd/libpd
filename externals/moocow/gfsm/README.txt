    README for pd external package 'gfsm'

    Last updated for pd-gfsm v0.05

DESCRIPTION
    pd-gfsm provides Pd bindings for the GFSM finite-state machine library.

PREREQUISITES
    Pd  Available from http://crca.ucsd.edu/~msp/software.html

    libgfsm >= v0.0.8-pre6 (optional)
        A local copy of the libgfsm source tree is included with this
        distribution, and should be built and used by default.

        Newer versions should be available from
        http://www.ling.uni-potsdam.de/~moocow/projects/gfsm

INSTALLATION
    Issue the following commands to the shell:

       cd DISTNAME-X.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and iohannes m zmoelnig.

KNOWN BUGS
    None known.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

