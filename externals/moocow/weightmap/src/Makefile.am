# File: ./src/Makefile.am
# Package: (pd external): weightmap
# Description:
#   + src-level automake file
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/common/pdexternal.am

## --- recursion subdirectories
#SUBDIRS = 

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = weightmap

## --- possible externals
EXTRA_PROGRAMS = \
	weightmap

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = weightmap-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

weightmap_SOURCES = weightmap.c $(top_srcdir)/common/mooPdUtils.h

AM_CPPFLAGS += -I$(top_srcdir)/common
