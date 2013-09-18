
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = filmQT.sln filmQT.vcproj

pkglib_LTLIBRARIES=
if HAVE_FRAMEWORK_QUICKTIME
pkglib_LTLIBRARIES+= gem_filmQT.la
endif

gem_filmQT_la_CXXFLAGS =
gem_filmQT_la_LDFLAGS  = -module -avoid-version -shared
gem_filmQT_la_LIBADD   =

# RTE
gem_filmQT_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmQT_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_filmQT_la_CXXFLAGS += @GEM_FRAMEWORK_QUICKTIME@
gem_filmQT_la_LIBADD   += @GEM_FRAMEWORK_QUICKTIME@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmQT_la_SOURCES= filmQT.cpp filmQT.h

