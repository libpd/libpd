# File: ./Makefile.am
# Package: hello
# Description:
#   + simple automake file for example pd external
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Includes
#-----------------------------------------------------------------------
include $(top_srcdir)/common/pdexternal.am

##-----------------------------------------------------------------------
## Options & Subdirectories
##-----------------------------------------------------------------------

##-- aclocal options (top level only)
ACLOCAL_AMFLAGS = -I common/m4

## --- automake options (top level only)
#AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip
AUTOMAKE_OPTIONS = foreign

## --- recursion subdirectories
#SUBDIRS = config src

##-----------------------------------------------------------------------
## Pd Externals: hacked _PROGRAMS
#-----------------------------------------------------------------------

pdexterns_PROGRAMS = hello
if WANT_OBJECT_EXTERNALS
 pdexterns_PROGRAMS += goodbye
endif

##-- possible single-object externals (as _PROGRAMS)
EXTRA_PROGRAMS = goodbye

##-- sources
hello_SOURCES = hello.c common/mooPdUtils.h
goodbye_SOURCES = goodbye.c common/mooPdUtils.h

##-- patches
pdexterns_DATA = heynow.pd

##-- documentation
pddoc_DATA = \
	hello-help.pd \
	goodbye-help.pd

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

all-local: $(PODS:.pod=.txt)

##-----------------------------------------------------------------------
## Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
#EXTRA_DIST  += $(PODS) $(PODS:.pod=.txt) ##-- included by default
EXTRA_DIST  += $(PODS:.pod=.html)
