
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = filmDS.sln filmDS.vcproj

pkglib_LTLIBRARIES=
if WINDOWS
pkglib_LTLIBRARIES+= gem_filmDS.la
endif

gem_filmDS_la_CXXFLAGS =
gem_filmDS_la_LDFLAGS  = -module -avoid-version -shared
gem_filmDS_la_LIBADD   =

# RTE
gem_filmDS_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmDS_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
# none

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmDS_la_SOURCES= filmDS.cpp filmDS.h

