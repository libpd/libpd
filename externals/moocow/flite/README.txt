    README for PD external distribution 'pd-flite'

    Last updated for version 0.01

DESCRIPTION
    The 'pd-flite' distribution contains a single PD external ("flite"),
    which provides a high-level text-to-speech interface for English based
    on the 'libflite' library by Alan W Black and Kevin A. Lenzo.

    Currently tested only under linux.

REQUIREMENTS
    * libflite >= v1.1
        The 'libflite' library by Alan W Black and Keven A. Lenzo is
        required to build the PD 'flite' external. It is available from
        http://cmuflite.org.

        You may want to apply the patch 'libflite-noaudio.patch' which comes
        with this distribution to the 'libflite' sources before compiling
        them (the '--with-audio=none' configure flag did not work for me on
        its own).

INSTALLATION
    First, build and install the libflite distribution.

    Then, issue the following commands to the shell:

       cd PACKAGENAME-X.YY  (or wherever you extracted this distribution)
       ./configure
       make
       make install

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    * --with-flite-dir=DIR
        Specify the base directory of the libflite distribution.

    * --with-pd-dir=DIR
        Specify PD base directory.

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

    * Environment Variables
        CPPFLAGS, CFLAGS, LDFLAGS, etc.

    See the output of './configure --help' for more options.

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    Flite run-time speech synthesis library by Alan W Black and Kevin A.
    Lenzo.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and iohannes m zmoelnig.

KNOWN BUGS
    It gobbles memory, and also processor time on synthesis operations.

    No support for alternative voices or lexica, and no mid- or low-level
    interface to the libflite functions.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

