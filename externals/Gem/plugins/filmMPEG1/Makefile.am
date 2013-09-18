
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=
if HAVE_LIB_MPEG
pkglib_LTLIBRARIES+= gem_filmMPEG1.la
endif

gem_filmMPEG1_la_CXXFLAGS =
gem_filmMPEG1_la_LDFLAGS  = -module -avoid-version -shared
gem_filmMPEG1_la_LIBADD   =

# RTE
gem_filmMPEG1_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmMPEG1_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_filmMPEG1_la_CXXFLAGS += @GEM_LIB_MPEG_CFLAGS@
gem_filmMPEG1_la_LIBADD   += @GEM_LIB_MPEG_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmMPEG1_la_SOURCES  = filmMPEG1.cpp filmMPEG1.h

