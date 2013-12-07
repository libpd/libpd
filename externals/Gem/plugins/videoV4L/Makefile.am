
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES= 

if HAVE_V4L
pkglib_LTLIBRARIES+=gem_videoV4L.la
endif

gem_videoV4L_la_CXXFLAGS =
gem_videoV4L_la_LDFLAGS  = -module -avoid-version -shared
gem_videoV4L_la_LIBADD   =

# RTE
gem_videoV4L_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoV4L_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_videoV4L_la_CXXFLAGS += @GEM_LIB_LIBV4L1_CFLAGS@
gem_videoV4L_la_LIBADD   += @GEM_LIB_LIBV4L1_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoV4L_la_SOURCES= videoV4L.cpp videoV4L.h

