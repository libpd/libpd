# File: ./src/Makefile.am
# Package: pd-gfsm
# Description:
#   + src-level automake file
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

## --- recursion subdirectories
#SUBDIRS = 

##-----------------------------------------------------------------------
## Pd Externals: hacked _PROGRAMS
##-----------------------------------------------------------------------

## --- externals
pdexterns_PROGRAMS = gfsm

## --- patches
pdexterns_DATA = \
	gfsm_markov.pd

## --- documentation
pddoc_DATA = \
	gfsm-help.pd \
	gfsm_alphabet-help.pd \
	gfsm_automaton-help.pd \
	gfsm_state-help.pd \
	gfsm_markov-help.pd

#-----------------------------------------------------------------------
# sources
#-----------------------------------------------------------------------

gfsm_SOURCES = \
	config.h \
	noconfig.h \
	atom_alphabet.h atom_alphabet.c \
	pd_alphabet.h  pd_alphabet.c \
	\
	pd_automaton.h pd_automaton.c \
	pd_algebra.c \
	\
	pd_state.h pd_state.c \
	\
	pd_io.h pd_io.c \
	\
	pd_gfsm.h pd_gfsm.c


noconfig.h: config.h.in
	rm -f $@
	cp $< $@
