    README for pd external package 'pdstring'

    Last updated for pdstring v0.06

DESCRIPTION
    The 'pdstring' package contains objects for converting to and from
    (ASCII)-strings, represented as lists of floats.

INSTALLATION
    Issue the following commands to the shell:

       cd PACKAGE-XX.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    * --help
        Output a brief usage summary of the 'configure' script, including a
        list of supported options and influential environment variables.

    * --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and iohannes m zmoelnig.

KNOWN BUGS
  Memory Usage
    Encoding each byte of a string as its own float is shamefully wasteful:
    it uses only 1 byte out of at least 3 which could be losslessly used
    given ANSI/IEEE Std 754-1985 floats, not to mention the remaining
    byte(s) (usually 1) of the float itself or the (usually 4) bytes used
    for the a_type flag. Unfortunately, Pd trims some floating point
    precision in message boxes and in float atoms, so a truly lossless float
    encoding for Pd would only be possible using 2 bytes per float (wasting
    1/2 the space of the float itself), and (to me), the memory saving such
    an encoding would provide is just not worth the lack of transparency and
    additional workload it would involve (but contact me if you want the
    code anyways).

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

