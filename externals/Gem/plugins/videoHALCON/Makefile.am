
ACLOCAL_AMFLAGS = -I ../../m4 -I .
AM_CPPFLAGS = -I$(srcdir)/../../src

EXTRA_DIST = halcon.m4 videoHALCON.sln videoHALCON.vcproj LICENSE.txt README.txt

plugindir=$(libdir)/Gem

plugin_LTLIBRARIES=
if HAVE_HALCON
plugin_LTLIBRARIES+= gem_videoHALCON.la
endif

gem_videoHALCON_la_CXXFLAGS =
gem_videoHALCON_la_LDFLAGS  = -module -avoid-version -shared
gem_videoHALCON_la_LIBADD   =

# RTE
gem_videoHALCON_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gem_videoHALCON_la_LDFLAGS  += @GEM_RTE_LIBS@
#gem_videoHALCON_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
#gem_videoHALCON_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# Dependencies
gem_videoHALCON_la_CXXFLAGS += @GEM_HALCON_CXXFLAGS@
gem_videoHALCON_la_LIBADD   += @GEM_HALCON_LIBS@

# convenience symlinks
include $(srcdir)/../symlink_ltlib.mk


### SOURCES
gem_videoHALCON_la_SOURCES= videoHALCON.cpp videoHALCON.h

