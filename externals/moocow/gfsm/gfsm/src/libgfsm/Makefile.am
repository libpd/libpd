## File: src/Makefile.am
## Package: gfsm
## Description:
##   + source-level automake file 
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## subdirectories
##-----------------------------------------------------------------------
SUBDIRS = gnulib

##-----------------------------------------------------------------------
## primary targets
##-----------------------------------------------------------------------

## --- executable programs
#bin_PROGRAMS = ${prog_1} ... ${prog_N}
#bin_PROGRAMS = dwdspp moot moot-fstgen moot-pargen

## --- libtool libraries
#lib_LTLIBRARIES = ${lib_1}.la ... ${lib_N}.la
#lib_LTLIBRARIES = libgfsm.la
noinst_LTLIBRARIES = libgfsm.la

## --- shared data in @pkgdatadir@
#pkgdata_DATA = ${data_1} ... ${data_N}

## --- manpages -- section is auto-detected
#man_MANS = ${man_1} ... ${man_N}

##-----------------------------------------------------------------------
## sources
##-----------------------------------------------------------------------

## --- Required sources
#${prog_i}_SOURCES =
#${lib_i}_la_SOURCES =
libgfsm_la_SOURCES = \
	config.h \
	gfsmCommon.c \
	gfsmCompound.c \
	gfsmDebug.c \
	gfsmError.c \
	gfsmIO.c \
	gfsmMem.c \
	gfsmVersion.c \
	gfsmUtils.c \
	gfsmEnum.c \
	gfsmSet.c \
	gfsmWeightMap.c \
	gfsmBitVector.c \
	gfsmAlphabet.c \
	gfsmSemiring.c \
	gfsmArc.c \
	gfsmArcList.c \
	gfsmArcIter.c \
	gfsmArcIndex.c \
	gfsmState.c \
	gfsmStateSet.c \
	gfsmAutomaton.c \
	gfsmAutomatonIO.c \
	gfsmDraw.c \
	gfsmAlgebra.c \
	gfsmArith.c \
	gfsmLookup.c \
	gfsmPaths.c \
	gfsmTrie.c \
	gfsmScanner.c \
	gfsmRegex.lex.l \
	gfsmRegex.tab.y \
	gfsmRegexCompiler.c \
	gfsmIndexed.c \
	gfsmIndexedIO.c

sources_argh = \
	gfsmIndexed.c \
	gfsmIndexed2.c \
	gfsmIndexedIO.c

## --- Extra (possible) sources
#EXTRA_${prog_i}_SOURCES = 
#EXTRA_${lib_i}_la_SOURCES = 

## --- Additional dependencies

##----------------------------------------------------
## Additional rules

#gfsmConfigNoAuto.h: gfsmConfigAuto.h.in
#	rm -f "$@"
#	cp "$<" "$@"

##-----------------------------------------------------------------------
## headers
##-----------------------------------------------------------------------

## --- installable headers: @includedir@/@pkgname@
#pkginclude_HEADERS = ${hfile_1} ... ${hfile_N}
#pkginclude_HEADERS =
noinst_HEADERS = \
	gfsmAssert.h \
	gfsmConfig.h \
	gfsmConfigNoAuto.h \
	gfsmConfigAuto.h \
	gfsmCommon.h \
	gfsmCompound.h \
	gfsmCompound.hi \
	gfsmDebug.h \
	gfsmError.h \
	gfsmIO.h \
	gfsmMem.h \
	gfsmMem.hi \
	gfsmVersion.h \
	gfsmUtils.h \
	gfsmEnum.h \
	gfsmEnum.hi \
	gfsmSet.h \
	gfsmSet.hi \
	gfsmWeightMap.h \
	gfsmWeightMap.hi \
	gfsmBitVector.h \
	gfsmBitVector.hi \
	gfsmAlphabet.h \
	gfsmSemiring.h \
	gfsmSemiring.hi \
	gfsmArc.h \
	gfsmArc.hi \
	gfsmArcList.h \
	gfsmArcList.hi \
	gfsmArcIter.h \
	gfsmArcIter.hi \
	gfsmArcIndex.h \
	gfsmArcIndex.hi \
	gfsmState.h \
	gfsmState.hi \
	gfsmStateSet.h \
	gfsmStateSet.hi \
	gfsmAutomaton.h \
	gfsmAutomaton.hi \
	gfsmAutomatonIO.h \
	gfsmDraw.h \
	gfsmAlgebra.h \
	gfsmArith.h \
	gfsmLookup.h \
	gfsmPaths.h \
	gfsmTrie.h \
	gfsmScanner.h \
	gfsmRegexCompiler.h \
	gfsmIndexed.h \
	gfsmIndexed.hi \
	gfsmIndexedIO.h \
	gfsm.h

headers_argh = \
	gfsmIndexed.h \
	gfsmIndexed2.h \
	gfsmIndexedIO.h

## --- no-install headers
#noinst_HEADERS = nopackage.h

##-----------------------------------------------------------------------
## pre-compile rules: flex ; bison
##-----------------------------------------------------------------------
SRCDIR = @srcdir@

if HAVE_FLEX
MY_LEX    = @FLEX@
MY_LFLAGS =
else
MY_LEX    = sh $(SRCDIR)/dummy-flex.sh
MY_LFLAGS =
endif
LEX       = $(MY_LEX)
AM_LFLAGS = $(MY_LFLAGS)

if HAVE_BISON
MY_YACC   = @BISON@
MY_YFLAGS = --defines --fixed-output-files --name-prefix="$(basename $*)_yy"
else
MY_YACC   = sh $(SRCDIR)/dummy-bison.sh
MY_YFLAGS =
endif
YACC      = $(MY_YACC)
AM_YFLAGS = $(MY_YFLAGS)

SUFFIXES = .l .lex.l .y .tab.y .tab.c .tab.h .lex.c .lex.h

##----------------------------------------
gfsmRegex.lex.o: gfsmRegex.lex.c gfsmRegex.tab.c gfsmRegexCompiler.h
gfsmRegex.tab.o: gfsmRegex.tab.c gfsmRegex.lex.c gfsmRegexCompiler.h
gfsmRegexCompiler.o: gfsmRegexCompiler.c gfsmRegexCompiler.h gfsmRegex.lex.c gfsmRegex.tab.c

##-----------------------------------------------------------------------
## compile flags
##-----------------------------------------------------------------------

##--- preprocessor flags
#AM_CPPFLAGS =
AM_CPPFLAGS = -I. -I$(SRCDIR)/gnulib

##--- compiler flags (cc)
AM_CFLAGS = $(gfsm_WFLAGS) $(gfsm_CFLAGS) $(gfsm_OFLAGS)

##--- linker flags
AM_LDFLAGS = $(gfsm_LDFLAGS)

##--- libraries
AM_LIBS = $(gfsm_LIBS)

##-----------------------------------------------------------------------
## Additional variable: library versioning
##  + for -version-info $(CUR):$(REV):$(AGE) :
##  + base lib is created as libMYLIB.so.$(CUR).$(AGE).$(REV)
##-----------------------------------------------------------------------

## --- The most recent interface number that this library implements.
LIBCUR = $(GFSM_VERSION_MAJOR)

## --- The difference between the newest and oldest interfaces that this
##     library implements.  In other words, the library implements all the
##     interface numbers in the range from number `CURRENT - AGE' to
##     `CURRENT'.
LIBAGE = 0

## --- The implementation number of the CURRENT interface.
LIBREV = $(GFSM_VERSION_MINOR)

##-----------------------------------------------------------------------
## linker flags
##   + library special handling of configure.in vars:
##-----------------------------------------------------------------------
#${prog_i}_LDFLAGS = -L. -static
#${prog_i}_LDADD   = ${non_src_file}.o -lsomelib

#${lib_i}_la_LDFLAGS = -L. -version-info ${lib_i_current}:${lib_i_rev}:${lib_i_age}
#${lib_i}_la_LIBADD  = -lotherlib

libgfsm_la_LDFLAGS = -version-info $(LIBCUR):$(LIBREV):$(LIBAGE)
#libgfsm_la_LIBADD  = gnulib/libgnu.la @gfsm_LIBS@ $(LIBS)

##-----------------------------------------------------------------------
## Variables: cleanup
##-----------------------------------------------------------------------

## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean:  built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
#DISTCLEANFILES =

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = \
	*~ .*~ Makefile Makefile.in \
	gmon.out

##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	config.h \
	gfsmConfigNoAuto.h \
	gfsmRegex.lex.c \
	gfsmRegex.lex.h \
	gfsmRegex.tab.c \
	gfsmRegex.tab.h \
	dummy-flex.sh \
	dummy-bison.sh

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

## --- dist-hook: when another 'Makefile.am' is overkill
#DISTHOOK_DIRS = foo
#DISTHOOK_FILES = foo/bar.txt foo/baz.txt
#dist-hook:
#	for d in $(DISTHOOK_DIRS); do\
#	  mkdir -p $(distdir)/$$d ;\
#	done
#	for f in $(DISTHOOK_FILES); do\
#	  cp -p $(srcdir)/$$f $(distdir)/$$f ;\
#	done

##-----------------------------------------------------------------------
## Rules: cleanup
##-----------------------------------------------------------------------
.PHONY: cvsclean

cvsclean: maintainer-clean ;
