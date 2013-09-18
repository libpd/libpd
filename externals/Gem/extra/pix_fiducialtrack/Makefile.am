AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I $(top_srcdir)/src/m4
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../convenience-link-extra.mk

EXTRA_DIST =
EXTRA_DIST +=pix_fiducialtrack.sln pix_fiducialtrack.vcproj

extradir=$(libdir)/pix_fiducialtrack

extra_LTLIBRARIES = pix_fiducialtrack.la

dist_extra_DATA  = 
dist_extra_DATA += pix_fiducialtrack-help.pd all.trees

## some default flags
pix_fiducialtrack_la_CXXFLAGS =
pix_fiducialtrack_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
pix_fiducialtrack_la_LIBADD   =

# RTE flags
pix_fiducialtrack_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
pix_fiducialtrack_la_LIBADD   += @GEM_RTE_LIBS@

pix_fiducialtrack_la_LDFLAGS += @GEM_ARCH_LDFLAGS@

### SOURCES
pix_fiducialtrack_la_SOURCES = \
	pix_fiducialtrack.cpp \
	pix_fiducialtrack.h \
	libfidtrack_fidtrackX.cpp \
	libfidtrack_fidtrackX.h \
	libfidtrack_segment.cpp \
	libfidtrack_segment.h \
	libfidtrack_treeidmap.cpp \
	libfidtrack_treeidmap.h
