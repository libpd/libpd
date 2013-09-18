ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST=videoVFW.sln videoVFW.vcproj

pkglib_LTLIBRARIES=
if WINDOWS
pkglib_LTLIBRARIES+= gem_videoVFW.la
endif

gem_videoVFW_la_CXXFLAGS =
gem_videoVFW_la_LDFLAGS  = -module -avoid-version -shared
gem_videoVFW_la_LIBADD   =

# RTE
gem_videoVFW_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoVFW_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_videoVFW_la_CXXFLAGS += @GEM_LIB_VFW32_CFLAGS@
gem_videoVFW_la_LIBADD   += @GEM_LIB_VFW32_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoVFW_la_SOURCES= videoVFW.cpp videoVFW.h

