
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=
if HAVE_LIB_LIBDC1394_2
pkglib_LTLIBRARIES+= gem_videoDC1394.la
endif

gem_videoDC1394_la_CXXFLAGS =
gem_videoDC1394_la_LDFLAGS  = -module -avoid-version -shared
gem_videoDC1394_la_LIBADD   =

# RTE
gem_videoDC1394_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoDC1394_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies

gem_videoDC1394_la_CXXFLAGS += @GEM_LIB_LIBDC1394_2_CFLAGS@
gem_videoDC1394_la_LIBADD   += @GEM_LIB_LIBDC1394_2_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoDC1394_la_SOURCES= videoDC1394.cpp videoDC1394.h

