#####################################################################
# Gem/RTE: encapsulation of the underlying RealTimeEnvironment
#          (currently only Pd)
#          this is just a start, lots of Pd-specific code is everywhere
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libRTE.la

libRTE_la_CXXFLAGS =
libRTE_la_LIBADD   =
libRTE_la_LDFLAGS  =

# RTE flags
libRTE_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libRTE_la_LIBADD   += @GEM_RTE_LIBS@
libRTE_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libRTE_la_includedir = $(includedir)/Gem/RTE
libRTE_la_include_HEADERS = \
	Array.h \
	MessageCallbacks.h



libRTE_la_SOURCES= \
        Array.cpp \
        Array.h \
        MessageCallbacks.h \
        Outlet.cpp \
        Outlet.h \
        Symbol.cpp \
        Symbol.h
