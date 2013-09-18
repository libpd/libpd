#####################################################################
# Gem/Output: objectclasses to handle windows
#####################################################################

AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

pkglib_LTLIBRARIES = 

#if HAVE_COCOA
if DISABLED
pkglib_LTLIBRARIES += gemcocoawindow.la
endif

if HAVE_GLUT
pkglib_LTLIBRARIES += gemglutwindow.la
endif

if HAVE_GLX
pkglib_LTLIBRARIES += gemglxwindow.la
endif

#if HAVE_CARBON
if DISABLED
pkglib_LTLIBRARIES += gemmacwindow.la
endif

if WINDOWS
pkglib_LTLIBRARIES += gemw32window.la
endif

########### gemcocoawindow ###########
# some default flags
gemcocoawindow_la_CXXFLAGS =
gemcocoawindow_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
gemcocoawindow_la_LIBADD   =
# RTE flags
gemcocoawindow_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gemcocoawindow_la_LIBADD   += @GEM_RTE_LIBS@
# arch flags
gemcocoawindow_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
gemcocoawindow_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# object specific libraries
gemcocoawindow_la_CXXFLAGS += 
gemcocoawindow_la_LIBADD   += 
gemcocoawindow_la_LDFLAGS  += 

## SOURCES
gemcocoawindow_la_SOURCES = \
  gemcocoawindow.mm \
  gemcocoawindow.h


########### gemglutwindow ###########
# some default flags
gemglutwindow_la_CXXFLAGS =
gemglutwindow_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
gemglutwindow_la_LIBADD   =
# RTE flags
gemglutwindow_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gemglutwindow_la_LIBADD   += @GEM_RTE_LIBS@
# arch flags
gemglutwindow_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
gemglutwindow_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# object specific libraries
gemglutwindow_la_CXXFLAGS += 
gemglutwindow_la_LIBADD   += 
gemglutwindow_la_LDFLAGS  += 

## SOURCES
gemglutwindow_la_SOURCES = \
  gemglutwindow.cpp \
  gemglutwindow.h


########### gemglxwindow ###########
# some default flags
gemglxwindow_la_CXXFLAGS =
gemglxwindow_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
gemglxwindow_la_LIBADD   =
# RTE flags
gemglxwindow_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gemglxwindow_la_LIBADD   += @GEM_RTE_LIBS@
# arch flags
gemglxwindow_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
gemglxwindow_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# object specific libraries
gemglxwindow_la_CXXFLAGS += $(X_CFLAGS)
gemglxwindow_la_LIBADD   += $(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS)
gemglxwindow_la_LDFLAGS  += 

## SOURCES
gemglxwindow_la_SOURCES = \
  gemglxwindow.cpp \
  gemglxwindow.h


########### gemmacwindow ###########
# some default flags
gemmacwindow_la_CXXFLAGS =
gemmacwindow_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
gemmacwindow_la_LIBADD   =
# RTE flags
gemmacwindow_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gemmacwindow_la_LIBADD   += @GEM_RTE_LIBS@
# arch flags
gemmacwindow_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
gemmacwindow_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# object specific libraries
gemmacwindow_la_CXXFLAGS += 
gemmacwindow_la_LIBADD   += 
gemmacwindow_la_LDFLAGS  += 

## SOURCES
gemmacwindow_la_SOURCES = \
  gemmacwindow.cpp \
  gemmacwindow.h


########### gemw32window ###########
# some default flags
gemw32window_la_CXXFLAGS =
gemw32window_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
gemw32window_la_LIBADD   =
# RTE flags
gemw32window_la_CXXFLAGS += @GEM_RTE_CFLAGS@
gemw32window_la_LIBADD   += @GEM_RTE_LIBS@
# arch flags
gemw32window_la_CXXFLAGS += @GEM_ARCH_CXXFLAGS@
gemw32window_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# object specific libraries
gemw32window_la_CXXFLAGS += 
gemw32window_la_LIBADD   += 
gemw32window_la_LDFLAGS  += 

## SOURCES
gemw32window_la_SOURCES = \
  gemw32window.cpp \
  gemw32window.h


