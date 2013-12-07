
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=

if VIDEODARWIN
pkglib_LTLIBRARIES+=gem_videoDarwin.la
endif

gem_videoDarwin_la_CXXFLAGS =
gem_videoDarwin_la_LDFLAGS  = -module -avoid-version -shared
gem_videoDarwin_la_LIBADD   =

# RTE
gem_videoDarwin_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoDarwin_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_videoDarwin_la_CXXFLAGS += 
gem_videoDarwin_la_LIBADD   += @GEM_FRAMEWORK_QUICKTIME@ @GEM_FRAMEWORK_CARBON@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoDarwin_la_SOURCES= videoDarwin.cpp videoDarwin.h


