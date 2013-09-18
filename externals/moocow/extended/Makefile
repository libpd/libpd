## -*- Mode: Makefile -*-
##
## File: externals/moocow/extended/Makefile
## Author: Bryan Jurish <moocow@bbaw.de>
## Description: pd-extended makefile for moocow's externals
##

##======================================================================
## Variables

SUBDIRS = \
	locale \
	pdstring \
	flite \
	deque \
	readdir \
	weightmap \
	sprinkler \
	gfsm

##-- 'pdstring' and 'sprinkler' are already in 'flatspace' ... should they be moved here?
##    -> 2008-08: removed any2string,string2any,pdstring from flatspace

##-- local variables
MOOCOW_DIR    ?=$(shell pwd)
MOOCOW_BUILD  ?=$(MOOCOW_DIR)/build.moo
MOOCOW_MFLAGS ?=DESTDIR=""
MOOCOW_BUILD_VERSION ?=2009-04-27.002

##-- arguments to be passed to sh ./configure
## + we use "`echo $(CFLAGS)`" to remove ugly quotes from the CFLAGS variable,
##   since autoconf doesn't parse these correctly (problem on win32 e.g. -D'O_NONBLOCK=1')
#	CFLAGS="$(CFLAGS)"   ##-- ?
#	CFLAGS="`echo $(CFLAGS)`" ##-- works
#	CFLAGS="$(subst ',,$(CFLAGS))" ##-- works
#	CFLAGS="$(shell echo $(CFLAGS))" ##-- works
CONFIGURE_ARGS=\
	CFLAGS="$(CFLAGS) -I/sw/include" \
	LDFLAGS="$(LDFLAGS) -L$(pd_src)/src -L$(pd_src)/bin -L$(pd_src)/obj -L/sw/lib" \
	--with-pd-include="$(pd_src)/src" \
	--with-pd-dir="$(MOOCOW_BUILD)" \
	--disable-dependency-tracking

#	--with-pd-extdir="$(MOOCOW_BUILD)/externs"

## ONFAIL
##  + set to 'true' to ignore failed sub-targets (for robust autobuild)
##  + set to 'false' to turn failed builds into make errors (maintainer only)
#ONFAIL ?= true
#ONFAIL ?= false
ONFAIL ?= echo "(MOOCOW) Warning: sub-target failed: '$@'"

##-- defaults
#CFLAGS ?= -g -O2 -Wall -Winline -W
CFLAGS ?= -g -O0 -Wall -D'mooUglyQuotedMacro(x)=x' -DmooSafeMacro=1

pd_src ?= $(CURDIR)/../../../pd

##-- LN_S : make a symlink (for common code)
#LN_S ?= ln -s
LN_S ?= cp -R

##======================================================================
## Rules: default
all: build.stamp

##======================================================================
## Rules: subdirectory: dummy

DUMMY_DIR            ?=../dummy
DUMMY_CONFIGURE_ARGS ?=--disable-shared
dummy.autogen_stamp: 
	test -e $(DUMMY_DIR)/common || $(LN_S) ../common $(DUMMY_DIR)/common || $(ONFAIL)
#	(cd $(DUMMY_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(DUMMY_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

dummy.configure_stamp: dummy.autogen_stamp
	(cd $(DUMMY_DIR) && sh ./configure $(CONFIGURE_ARGS) $(DUMMY_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

dummy.build_stamp: dummy.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(DUMMY_DIR) all install || $(ONFAIL)
	touch $@

dummy.extclean:
	rm -f dummy.autogen_stamp dummy.configure_stamp dummy.build_stamp

dummy.clean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) clean || $(ONFAIL)

dummy.distclean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) distclean || $(ONFAIL)

dummy.cvsclean: dummy.extclean
	$(MAKE) -C $(DUMMY_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: flite

FLITE_DIR            ?=../flite
#FLITE_CONFIGURE_ARGS ?=--with-flite-dir=/NOPE
#FLITE_CONFIGURE_ARGS ?=

flite.autogen_stamp: 
	test -e $(FLITE_DIR)/common || $(LN_S) ../common $(FLITE_DIR)/common || $(ONFAIL)
#	(cd $(FLITE_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(FLITE_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

flite.configure_stamp: flite.autogen_stamp
	(cd $(FLITE_DIR) && sh ./configure $(CONFIGURE_ARGS) $(FLITE_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

flite.build_stamp: flite.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(FLITE_DIR) all install || $(ONFAIL)
	touch $@

flite.extclean:
	rm -f flite.autogen_stamp flite.configure_stamp flite.build_stamp

flite.clean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) clean || $(ONFAIL)

flite.distclean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) distclean || $(ONFAIL)

flite.cvsclean: flite.extclean
	$(MAKE) -C $(FLITE_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: gfsm

GFSM_DIR            ?=../gfsm
#GFSM_CONFIGURE_ARGS ?=--disable-gfsm
#GFSM_CONFIGURE_ARGS ?=

gfsm.autogen_stamp: 
	test -e $(GFSM_DIR)/common || $(LN_S) ../common $(GFSM_DIR)/common || $(ONFAIL)
#	(cd $(GFSM_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(GFSM_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

gfsm.configure_stamp: gfsm.autogen_stamp
	(cd $(GFSM_DIR) && sh ./configure $(CONFIGURE_ARGS) $(GFSM_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

gfsm.build_stamp: gfsm.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(GFSM_DIR) all install || $(ONFAIL)
	touch $@

gfsm.extclean:
	rm -f gfsm.autogen_stamp gfsm.configure_stamp gfsm.build_stamp

gfsm.clean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) clean || $(ONFAIL)

gfsm.distclean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) distclean || $(ONFAIL)

gfsm.cvsclean: gfsm.extclean
	$(MAKE) -C $(GFSM_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: deque

DEQUE_DIR            ?=../deque
DEQUE_CONFIGURE_ARGS ?=
deque.autogen_stamp: 
	test -e $(DEQUE_DIR)/common || $(LN_S) ../common $(DEQUE_DIR)/common || $(ONFAIL)
#	(cd $(DEQUE_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(DEQUE_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

deque.configure_stamp: deque.autogen_stamp
	(cd $(DEQUE_DIR) && sh ./configure $(CONFIGURE_ARGS) $(DEQUE_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

deque.build_stamp: deque.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(DEQUE_DIR) all install || $(ONFAIL)
	touch $@

deque.extclean:
	rm -f deque.autogen_stamp deque.configure_stamp deque.build_stamp

deque.clean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) clean || $(ONFAIL)

deque.distclean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) distclean || $(ONFAIL)

deque.cvsclean: deque.extclean
	$(MAKE) -C $(DEQUE_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: locale

LOCALE_DIR            ?=../locale
#LOCALE_CONFIGURE_ARGS ?=
#LOCALE_CONFIGURE_ARGS ?=

locale.autogen_stamp: 
	test -e $(LOCALE_DIR)/common || $(LN_S) ../common $(LOCALE_DIR)/common || $(ONFAIL)
#	(cd $(LOCALE_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(LOCALE_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

locale.configure_stamp: locale.autogen_stamp
	(cd $(LOCALE_DIR) && sh ./configure $(CONFIGURE_ARGS) $(LOCALE_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

locale.build_stamp: locale.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(LOCALE_DIR) all install || $(ONFAIL)
	touch $@

locale.extclean:
	rm -f locale.autogen_stamp locale.configure_stamp locale.build_stamp

locale.clean: locale.extclean
	$(MAKE) -C $(LOCALE_DIR) clean || $(ONFAIL)

locale.distclean: locale.extclean
	$(MAKE) -C $(LOCALE_DIR) distclean || $(ONFAIL)

locale.cvsclean: locale.extclean
	$(MAKE) -C $(LOCALE_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: pdstring

PDSTRING_DIR            ?=../pdstring
PDSTRING_CONFIGURE_ARGS ?=--enable-object-externals
pdstring.autogen_stamp: 
	test -e $(PDSTRING_DIR)/common || $(LN_S) ../common $(PDSTRING_DIR)/common || $(ONFAIL)
#	(cd $(PDSTRING_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(PDSTRING_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

pdstring.configure_stamp: pdstring.autogen_stamp
	(cd $(PDSTRING_DIR) && sh ./configure $(CONFIGURE_ARGS) $(PDSTRING_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

pdstring.build_stamp: pdstring.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(PDSTRING_DIR) all install || $(ONFAIL)
	touch $@

pdstring.extclean:
	rm -f pdstring.autogen_stamp pdstring.configure_stamp pdstring.build_stamp

pdstring.clean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) clean || $(ONFAIL)

pdstring.distclean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) distclean || $(ONFAIL)

pdstring.cvsclean: pdstring.extclean
	$(MAKE) -C $(PDSTRING_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: readdir

READDIR_DIR            ?=../readdir
READDIR_CONFIGURE_ARGS ?=
readdir.autogen_stamp: 
	test -e $(READDIR_DIR)/common || $(LN_S) ../common $(READDIR_DIR)/common || $(ONFAIL)
#	(cd $(READDIR_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(READDIR_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

readdir.configure_stamp: readdir.autogen_stamp
	(cd $(READDIR_DIR) && sh ./configure $(CONFIGURE_ARGS) $(READDIR_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

readdir.build_stamp: readdir.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(READDIR_DIR) all install || $(ONFAIL)
	touch $@

readdir.extclean:
	rm -f readdir.autogen_stamp readdir.configure_stamp readdir.build_stamp

readdir.clean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) clean || $(ONFAIL)

readdir.distclean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) distclean || $(ONFAIL)

readdir.cvsclean: readdir.extclean
	$(MAKE) -C $(READDIR_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: sprinkler
##  + (used to be one directory higher in the tree than usual...)

SPRINKLER_DIR            ?=../sprinkler
SPRINKLER_CONFIGURE_ARGS ?=
sprinkler.autogen_stamp: 
	test -e $(SPRINKLER_DIR)/common || $(LN_S) ../common $(SPRINKLER_DIR)/common || $(ONFAIL)
#	(cd $(SPRINKLER_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(SPRINKLER_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

sprinkler.configure_stamp: sprinkler.autogen_stamp
	(cd $(SPRINKLER_DIR) && sh ./configure $(CONFIGURE_ARGS) $(SPRINKLER_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

sprinkler.build_stamp: sprinkler.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(SPRINKLER_DIR) all install || $(ONFAIL)
	touch $@

sprinkler.extclean:
	rm -f sprinkler.autogen_stamp sprinkler.configure_stamp sprinkler.build_stamp

sprinkler.clean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) clean || $(ONFAIL)

sprinkler.distclean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) distclean || $(ONFAIL)

sprinkler.cvsclean: sprinkler.extclean
	$(MAKE) -C $(SPRINKLER_DIR) cvsclean || $(ONFAIL)

##======================================================================
## Rules: subdirectory: weightmap

WEIGHTMAP_DIR            ?=../weightmap
WEIGHTMAP_CONFIGURE_ARGS ?=
weightmap.autogen_stamp: 
	test -e $(WEIGHTMAP_DIR)/common || $(LN_S) ../common $(WEIGHTMAP_DIR)/common || $(ONFAIL)
#	(cd $(WEIGHTMAP_DIR); sh ./svn-prepare.sh copy) || $(ONFAIL)
#	(cd $(WEIGHTMAP_DIR); sh ./autogen.sh) || $(ONFAIL)
	touch $@

weightmap.configure_stamp: weightmap.autogen_stamp
	(cd $(WEIGHTMAP_DIR) && sh ./configure $(CONFIGURE_ARGS) $(WEIGHTMAP_CONFIGURE_ARGS) && make clean) || $(ONFAIL)
	touch $@

weightmap.build_stamp: weightmap.configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(WEIGHTMAP_DIR) all install || $(ONFAIL)
	touch $@

weightmap.extclean:
	rm -f weightmap.autogen_stamp weightmap.configure_stamp weightmap.build_stamp

weightmap.clean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) clean || $(ONFAIL)

weightmap.distclean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) distclean || $(ONFAIL)

weightmap.cvsclean: weightmap.extclean
	$(MAKE) -C $(WEIGHTMAP_DIR) cvsclean || $(ONFAIL)


##======================================================================
## Rules: local
autogen: $(SUBDIRS:=.autogen_stamp)
configure: $(SUBDIRS:=.configure_stamp)

config:
	@echo "(moocow:debug) MOOCOW_BUILD_VERSION=$(MOOCOW_BUILD_VERSION)"

build.stamp: config $(SUBDIRS:=.build_stamp)
	rm -rf $(MOOCOW_BUILD)/noinstall
	touch $@

readme: README.txt
README.txt: README.pod
	pod2text README.pod $@

extclean: $(SUBDIRS:=.extclean)
	rm -f build.stamp config.log
	rm -rf $(MOOCOW_BUILD)

clean: $(SUBDIRS:=.clean) extclean

realclean: distclean

distclean: $(SUBDIRS:=.distclean) clean

cvsclean: $(SUBDIRS:=.cvsclean) clean
