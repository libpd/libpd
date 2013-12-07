    README for weightmap

    Last updated for weightmap v0.02

DESCRIPTION
    weightmap is a PD external which maps incoming probability values to
    integers, inspired in part by Yves Degoyon's 'probalizer' object.

INSTALLATION
    Issue the following commands to the shell:

       cd weightmap-X.YY  (or wherever you extracted the distribution)
       ./configure
       make
       make install

ACKNOWLEDGEMENTS
    PD by Miller Puckette and others.

    probalizer object by Yves Degoyon.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, Larry Troxler, and iohannes m zmoelnig.

BUGS
    It's a misleading name: higher input "weights" don't neccesarily
    correspond to higer stored "weights".

    Probably many more serious ones as well.

AUTHOR
    Bryan Jurish <moocow@ling.uni-potsdam.de>

