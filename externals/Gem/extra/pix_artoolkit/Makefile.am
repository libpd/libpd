ACLOCAL_AMFLAGS = -I ../../m4
AM_CPPFLAGS = -I$(srcdir)/../../src

EXTRA_DIST =
EXTRA_DIST += pix_artoolkit.sln  pix_artoolkit.vcproj autogen.sh

extradir=$(libdir)/pix_artoolkit
dist_extra_DATA  = 

extra_LTLIBRARIES  =
if HAVE_LIB_ARTOOLKIT
extra_LTLIBRARIES += pix_artoolkit.la

#dist_extra_DATA += LICENSE.txt
dist_extra_DATA += README.txt
dist_extra_DATA += pix_artoolkit-help.pd
dist_extra_DATA += pattHiro.pdf patt.hiro camera_para.dat
endif

## some default flags
pix_artoolkit_la_CXXFLAGS =
pix_artoolkit_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
pix_artoolkit_la_LIBADD   =

# RTE flags
pix_artoolkit_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
pix_artoolkit_la_LIBADD   += @GEM_RTE_LIBS@

pix_artoolkit_la_LDFLAGS += @GEM_ARCH_LDFLAGS@


# convenience symlinks
## uha, this is ugly
%.@GEM_RTE_EXTENSION@: %.la
	rm -f $@
	test -f .libs/$@ && $(LN_S) .libs/$@ $@ || true

.PHONY: clean-conviencesymlink

clean-conviencesymlink:
	rm -f *.@GEM_RTE_EXTENSION@


all-local:: $(extra_LTLIBRARIES:.la=.@GEM_RTE_EXTENSION@)

clean-local:: clean-conviencesymlink


### SOURCES
pix_artoolkit_la_SOURCES = \
	pix_artoolkit.cpp \
	pix_artoolkit.h
