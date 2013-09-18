AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I $(top_srcdir)/src/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST =
EXTRA_DIST +=pix_drum.sln pix_drum.vcproj

extradir=$(libdir)/pix_drum

extra_LTLIBRARIES = pix_drum.la

dist_extra_DATA  = LICENSE.txt
dist_extra_DATA += pix_drum-help.pd

## some default flags
pix_drum_la_CXXFLAGS =
pix_drum_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
pix_drum_la_LIBADD   =

# RTE flags
pix_drum_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
pix_drum_la_LIBADD   += @GEM_RTE_LIBS@

pix_drum_la_LDFLAGS += @GEM_ARCH_LDFLAGS@


# convenience symlinks
## uha, this is ugly
pix_drum.@GEM_RTE_EXTENSION@: pix_drum.la
	rm -f $@
	test -f .libs/$@ && $(LN_S) .libs/$@ $@

.PHONY: clean-conviencesymlink

clean-conviencesymlink:
	rm -f pix_drum.@GEM_RTE_EXTENSION@


all-local:: pix_drum.@GEM_RTE_EXTENSION@

clean-local:: clean-conviencesymlink


### SOURCES
pix_drum_la_SOURCES = \
	pix_drum.cpp \
	pix_drum.h

