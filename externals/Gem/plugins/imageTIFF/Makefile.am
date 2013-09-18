
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = imageTIFF.sln imageTIFF.vcproj

pkglib_LTLIBRARIES=
if HAVE_LIB_TIFF
pkglib_LTLIBRARIES+= gem_imageTIFF.la
endif


gem_imageTIFF_la_CXXFLAGS =
gem_imageTIFF_la_LDFLAGS  = -module -avoid-version -shared
gem_imageTIFF_la_LIBADD   =

# RTE
gem_imageTIFF_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_imageTIFF_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_imageTIFF_la_CXXFLAGS += @GEM_LIB_TIFF_CFLAGS@
gem_imageTIFF_la_LIBADD   += @GEM_LIB_TIFF_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_imageTIFF_la_SOURCES= imageTIFF.cpp imageTIFF.h

