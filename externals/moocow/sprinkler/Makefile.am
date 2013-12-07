# File: ./Makefile.am
# Package: sprinkler
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
#SUBDIRS = config src

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

all-local: $(PODS:.pod=.txt)

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = sprinkler

## --- possible externals
EXTRA_PROGRAMS = \
	sprinkler

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = sprinkler-help.pd

#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

sprinkler_SOURCES = sprinkler.c common/mooPdUtils.h

AM_CPPFLAGS += -I$(top_srcdir)/common
