#####################################################################
# Gem/Nongeos: objectclasses for 3D-objects that are not visible
#              (mainly lights)
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libNongeos.la

libNongeos_la_CXXFLAGS =
libNongeos_la_LIBADD   =
libNongeos_la_LDFLAGS  =

# RTE flags
libNongeos_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libNongeos_la_LIBADD   += @GEM_RTE_LIBS@
libNongeos_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libNongeos_la_SOURCES=  \
    light.cpp \
    light.h \
    spot_light.cpp \
    spot_light.h \
    world_light.cpp \
    world_light.h

