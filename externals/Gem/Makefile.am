AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS =src abstractions
SUBDIRS+=help doc examples
SUBDIRS+=build

## plugins
SUBDIRS+=plugins

## extra objects
SUBDIRS+=extra

#pkg-config support
## this is a bit hacky: we abused $(libdir) to point to .../lib/pd/extra
## so we cannot use it for the pkgconfigdir
## instead, we have to build it crom scratch
#pkgconfigdir = $(libdir)/pkgconfig
pkgconfigdir = ${exec_prefix}/lib/pkgconfig
pkgconfig_DATA = Gem.pc

gemdir=$(pkglibdir)

dist_gem_DATA = \
	ChangeLog \
	COPYING.txt \
	GnuGPL.LICENSE.txt \
	README.txt

EXTRA_DIST = autogen.sh
