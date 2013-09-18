ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = videoDS.sln videoDS.vcproj

pkglib_LTLIBRARIES=
if WINDOWS
pkglib_LTLIBRARIES+= gem_videoDS.la
endif

gem_videoDS_la_CXXFLAGS =
gem_videoDS_la_LDFLAGS  = -module -avoid-version -shared
gem_videoDS_la_LIBADD   =

# RTE
gem_videoDS_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoDS_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
## none

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoDS_la_SOURCES= videoDS.cpp videoDS.h DSgrabber.cpp DSgrabber.h

