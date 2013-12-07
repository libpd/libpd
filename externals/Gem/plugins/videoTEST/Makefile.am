
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES= 

pkglib_LTLIBRARIES+=gem_videoTEST.la

gem_videoTEST_la_CXXFLAGS =
gem_videoTEST_la_LDFLAGS  = -module -avoid-version -shared
gem_videoTEST_la_LIBADD   =

# RTE
gem_videoTEST_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoTEST_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoTEST_la_SOURCES= videoTEST.cpp videoTEST.h

