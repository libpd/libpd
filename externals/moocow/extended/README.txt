    README for externals/moocow/extended/ build hacks.

    Last updated Fri, 28 Nov 2008 22:00:13 +0100

DESCRIPTION
    This directory is for pd-extended compatible builds of (some of)
    moocow's externals directly from the CVS repository.

USAGE
    Issuing the following commands to the shell:

      cd externals/moocow/extended (or wherever you extracted the distribution)
      make

    ... should result in all objects being compiled into
    extended/build/externs. This is intended to be called from
    externals/Makefile.

SUPPORTED EXTERNALS
  Standard Externals
    The following externals are built by default:

     deque
     pdstring    (just the dummy object, not the library!)
      any2string
      string2any
     readdir
     sprinkler
     weightmap

  Optional Externals
    The following externals depend on additional libraries, which may or may
    not be installed on your system. The build procedures for these
    externals will be called, but may fail:

     flite
     gfsm

  Unsupported Externals
    The following externals and libraries are unsupported for various
    reasons:

     ratts

    See http://www.ling.uni-potsdam.de/~moocow/projects/pd for the
    "official" distributions.

ACKNOWLEDGEMENTS
    Pd by Miller Puckette and others.

    Ideas, black magic, and other nuggets of information drawn from code by
    Guenter Geiger, iohannes m zmoelnig, Hans-Christoph Steiner, and others.

KNOWN BUGS
    None known.

AUTHOR
    Bryan Jurish <moocow@bbaw.de>

