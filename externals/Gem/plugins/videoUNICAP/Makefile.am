
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=
if HAVE_LIB_LIBUNICAP
pkglib_LTLIBRARIES+= gem_videoUNICAP.la
endif

gem_videoUNICAP_la_CXXFLAGS =
gem_videoUNICAP_la_LDFLAGS  = -module -avoid-version -shared
gem_videoUNICAP_la_LIBADD   =

# RTE
gem_videoUNICAP_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_videoUNICAP_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_videoUNICAP_la_CXXFLAGS += @GEM_LIB_LIBUNICAP_CFLAGS@ 
gem_videoUNICAP_la_LIBADD   += @GEM_LIB_LIBUNICAP_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoUNICAP_la_SOURCES= videoUNICAP.cpp videoUNICAP.h

