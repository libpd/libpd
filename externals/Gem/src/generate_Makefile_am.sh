#!/bin/sh

TEMPLATEFILE=Makefile.am.template

if [ -e "${TEMPLATEFILE}" ]; then
 :
else
 LIBRARYNAME=$(pwd | sed -e 's|^.*/||')

echo '
AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)

noinst_LTLIBRARIES = lib@LTLIBRARYNAME@.la

libNongeos_la_CXXFLAGS =
libNongeos_la_LIBADD   =
libNongeos_la_LDFLAGS  =

# RTE flags
libNongeos_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libNongeos_la_LIBADD   += @GEM_RTE_LIBS@
libNongeos_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# Dependencies
## none

lib@LTLIBRARYNAME@_la_SOURCES= @SOURCES@
'| sed "s|@LTLIBRARYNAME@|${LIBRARYNAME}|g" > ${TEMPLATEFILE}

fi

echo "# this file was generated automatically from ${TEMPLATEFILE}
# rather than editing this file directly, you should instead edit the
# original ${TEMPLATEFILE} and re-generate this file" | \
cat - ${TEMPLATEFILE} | \
sed -e s/@SOURCES@/"$(ls *.cpp *.h | sort | sed -e 's/^/ \\\\\\n    /g' | tr -d '\n')"/
