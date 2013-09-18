
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = filmAVI.sln filmAVI.vcproj

pkglib_LTLIBRARIES=
if WINDOWS
pkglib_LTLIBRARIES += gem_filmAVI.la
endif

gem_filmAVI_la_CXXFLAGS =
gem_filmAVI_la_LDFLAGS  = -module -avoid-version -shared
gem_filmAVI_la_LIBADD   =

# RTE
gem_filmAVI_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmAVI_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
## none

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmAVI_la_SOURCES  = filmAVI.cpp filmAVI.h

