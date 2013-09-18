AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I $(top_srcdir)/src/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST =
EXTRA_DIST +=pix_mano.sln pix_mano.vcproj

extradir=$(libdir)/pix_mano

extra_LTLIBRARIES = pix_mano.la
 
dist_extra_DATA  = LICENSE.txt
dist_extra_DATA += pix_mano-help.pd


## some default flags
pix_mano_la_CXXFLAGS =
pix_mano_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
pix_mano_la_LIBADD   =

# RTE flags
pix_mano_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
pix_mano_la_LIBADD   += @GEM_RTE_LIBS@

pix_mano_la_LDFLAGS += @GEM_ARCH_LDFLAGS@


# convenience symlinks
## uha, this is ugly
pix_mano.@GEM_RTE_EXTENSION@: pix_mano.la
	rm -f $@
	test -f .libs/$@ && $(LN_S) .libs/$@ $@

.PHONY: clean-conviencesymlink

clean-conviencesymlink:
	rm -f pix_mano.@GEM_RTE_EXTENSION@


all-local:: pix_mano.@GEM_RTE_EXTENSION@

clean-local:: clean-conviencesymlink


### SOURCES
pix_mano_la_SOURCES = \
	pix_mano.cpp \
	pix_mano.h

