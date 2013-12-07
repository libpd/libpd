
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=

if HAVE_LIB_LIBQUICKTIME
pkglib_LTLIBRARIES+= gem_filmQT4L.la
endif
if HAVE_LIB_LQT
pkglib_LTLIBRARIES+= gem_filmQT4L.la
endif

gem_filmQT4L_la_CXXFLAGS =
gem_filmQT4L_la_LDFLAGS  = -module -avoid-version -shared
gem_filmQT4L_la_LIBADD   =

# RTE
gem_filmQT4L_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmQT4L_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_filmQT4L_la_CXXFLAGS += @GEM_LIB_LIBQUICKTIME_CFLAGS@ @GEM_LIB_LQT_CFLAGS@
gem_filmQT4L_la_LIBADD   += @GEM_LIB_LIBQUICKTIME_LIBS@   @GEM_LIB_LQT_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmQT4L_la_SOURCES= filmQT4L.cpp filmQT4L.h

