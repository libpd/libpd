    README for pd external 'locale'

    Last updated for locale v0.01

DESCRIPTION
    The 'locale' object provides access to the internationalization
    facilities in locale.h

INSTALLATION
    Issue the following commands to the shell:

       cd locale-X.YY  (or wherever you extracted the distribution)
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
    Injudicious use of the "set" method can confuse Pd float-parsing
    mechanisms. See locale-help.pd for details.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

