
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=
if IRIX
pkglib_LTLIBRARIES+=gem_videoSGI.la
endif

gem_videoSGI_la_CXXFLAGS =
gem_videoSGI_la_LDFLAGS  = -module -avoid-version -shared
gem_videoSGI_la_LIBADD   =

# RTE
gem_videoSGI_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoSGI_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
## none

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoSGI_la_SOURCES= videoSGI.cpp videoSGI.h

