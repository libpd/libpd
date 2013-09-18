# File: ./Makefile.am
# Package: flite
# Description:
#   + top-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/common/pdexternal.am

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

##-- aclocal options (top level only)
ACLOCAL_AMFLAGS = -I common/m4 

## --- automake options
#AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip
AUTOMAKE_OPTIONS = foreign

## --- recursion subdirectories
SUBDIRS = config

##-----------------------------------------------------------------------
## Pd Externals: hacked _PROGRAMS
#-----------------------------------------------------------------------

## --- externals
pdext_PROGRAMS = flite

## --- documentation
pddoc_DATA = flite-help.pd

#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------
flite_SOURCES = flite.c common/mooPdUtils.h

#-----------------------------------------------------------------------
# external compilation : flags
#-----------------------------------------------------------------------

flite_LDFLAGS = $(LFLAGS)
#flite_LDADD   = $(FLITE_VOICELIBS) $(FLITE_LANGLIBS) $(FLITE_LEXLIBS) $(FLITE_LIBS)

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

all-local: $(PODS:.pod=.txt)

#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST += flite-1.1-noaudio.patch

