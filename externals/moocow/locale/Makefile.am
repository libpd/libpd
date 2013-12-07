# File: ./Makefile.am
# Package: readdir
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
SUBDIRS = config src

#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

all-local: $(PODS:.pod=.txt)
