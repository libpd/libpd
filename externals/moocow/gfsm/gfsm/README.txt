    README for package gfsm

    Last updated for gfsm v0.0.8

DESCRIPTION
    The gfsm package consists of libgfsm, an abstract C library of tools for
    manipulation of finite state machines, and the gfsm utilities, a suite
    of command-line tools built on libgfsm. gfsm currently supports AT&T
    style weighted transducer text input and label definition files, as well
    as a number of common algebraic operations on finite state machines.

DEPENDENCIES
    glib-2.0 (REQUIRED)
        Available from: http://www.gtk.org/

        C library for common data structures. Tested versions 2.4.6, 2.8.3.

    zlib (Optional)
        Compression library by Jean-loup Gailly and Mark Adler which can be
        used for transparent (de)compression. Available from:
        http://www.gzip.org/zlib

        Tested version 1.2.1.

    pkg-config
        Available from: http://www.freedesktop.org/software/pkgconfig/

        To build from CVS, you will also need the pkg-config autoconf macros
        which come with the source distribution of pkg-config.

    doxygen (Optional)
        Required for building library documentation. Available from:
        http://www.doxygen.org

        Tested versions 1.2.15, 1.3.8, 1.4.4.

    Perl (Optional)
        Get it from http://www.cpan.org or http://www.perl.com

        Required for re-building command-line parsers and/or non-standard
        documentation formats.

    Getopt::Gen (Optional)
        A Perl module used to generate command-line option parsers. Should
        be available from the author of this package at:

        http://www.ling.uni-potsdam.de/~moocow/projects/perl

        Tested versions 0.09, 0.10.

        Note that Getopt::Gen depends on several 3rd-party perl modules,
        including Text::Template and Parse::Lex, which are available from
        CPAN.

        Note additionally that Parse::Lex v2.15 is broken: if it gives you
        grief, use the hacked version available at:

        http://www.ling.uni-potsdam.de/~moocow/projects/perl/ParseLex-2.15-h
        acked.tar.gz

    pod2man, pod2text, pod2html, pod2latex (Optional)
        The Perl documentation converstion utilities, required for building
        the correspdonding program documentation formats, should have come
        with your Perl. These are only required if you wish to build program
        documentation formats other than the defaults which come with the
        distribution.

INSTALLATION
    Issue the following commands to the shell:

     cd gfsm-X.Y.Z  (or wherever you extracted the distribution)
     sh ./configure
     make
     make install

  Additional installation targets
    The build system supports the following optional installation targets;
    call them with:

     make SOME_TARGET

    from the distribution root directory.

    install-magic
        Adds recognition support for stored binary gfsm files to the
        database for the file(1) utility. If your file(1) database lives
        somewhere other than /etc/magic (see magic(5) for details), call
        this target as:

         make magic=/full/path/to/my/file/magic install-magic

    uninstall-magic
        Removes gfsm recognition support (if present) from your /etc/magic
        database, or from whatever database you specified with the 'magic'
        variable.

BUILD OPTIONS
    The 'configure' script supports the following options, among others:

    --enable-debug , --disable-debug
        Whether to enable verbose debugging messages. Default=no.

    See the output of `./configure --help` for details on additional
    supported options.

ACKNOWLEDGEMENTS
    Ideas and code adapted from the SFST package by Helmut Schmid.

    Many thanks to Thomas Hanneforth for useful advice.

KNOWN BUGS
    Many.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

