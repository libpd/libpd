# File: ./src/Makefile.am
# Package: (pd external)
# Description:
#   + src-level automake file
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/common/pdexternal.am

#-----------------------------------------------------------------------
# pd externals (hacked _PROGRAMS target)
#-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = locale uselocale

## --- possible externals
EXTRA_PROGRAMS = \
	locale uselocale

## --- patches
pdexterns_DATA =

## --- documentation
pddoc_DATA = locale-help.pd


#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------
locale_SOURCES = locale.c
uselocale_SOURCES = uselocale.c
