
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = imageSGI.sln imageSGI.vcproj

pkglib_LTLIBRARIES= gem_imageSGI.la

gem_imageSGI_la_CXXFLAGS =
gem_imageSGI_la_LDFLAGS  = -module -avoid-version -shared
gem_imageSGI_la_LIBADD   =

# RTE
gem_imageSGI_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_imageSGI_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
## none

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_imageSGI_la_SOURCES= imageSGI.cpp imageSGI.h sgiimage.cpp sgiimage.h

