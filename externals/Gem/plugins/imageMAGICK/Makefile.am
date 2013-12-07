
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

EXTRA_DIST = imageMAGICK.sln imageMAGICK.vcproj

pkglib_LTLIBRARIES=
if HAVE_LIB_IMAGEMAGICK__
pkglib_LTLIBRARIES+= gem_imageMAGICK.la
endif

gem_imageMAGICK_la_CXXFLAGS =
gem_imageMAGICK_la_LDFLAGS  = -module -avoid-version -shared
gem_imageMAGICK_la_LIBADD   =

# RTE
gem_imageMAGICK_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_imageMAGICK_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_imageMAGICK_la_CXXFLAGS += @GEM_LIB_IMAGEMAGICK___CFLAGS@
gem_imageMAGICK_la_LIBADD   += @GEM_LIB_IMAGEMAGICK___LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_imageMAGICK_la_SOURCES= imageMAGICK.cpp imageMAGICK.h

