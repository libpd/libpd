
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src

pkglib_LTLIBRARIES=
if MACOSX
pkglib_LTLIBRARIES+= gem_filmDarwin.la
endif

gem_filmDarwin_la_CXXFLAGS =
gem_filmDarwin_la_LDFLAGS  = -module -avoid-version -shared
gem_filmDarwin_la_LIBADD   =

# RTE
gem_filmDarwin_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
gem_filmDarwin_la_LDFLAGS  += @GEM_RTE_LIBS@   @GEM_ARCH_LDFLAGS@

# Dependencies
gem_filmDarwin_la_CXXFLAGS += @GEM_FRAMEWORK_QUICKTIME@ @GEM_FRAMEWORK_CARBON@ 
gem_filmDarwin_la_LIBADD   += @GEM_FRAMEWORK_QUICKTIME@ @GEM_FRAMEWORK_CARBON@ 

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_filmDarwin_la_SOURCES= filmDarwin.cpp filmDarwin.h

