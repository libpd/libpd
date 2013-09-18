## -*- Mode: Makefile -*-
##
## File: externals/moocow/extended/Makefile
## Author: Bryan Jurish <moocow@bbaw.de>
## Description: pd-extended makefile for moocow's externals
##

##======================================================================
## Variables

SUBDIRS = \
	flite \
	gfsm \
	deque \
	readdir \
	weightmap \
	pdstring \
	sprinkler

##-- 'pdstring' and 'sprinkler' are already in 'flatspace' ... should they be moved here?
##    -> 2008-08: removed any2string,string2any,pdstring from flatspace

##-- local variables
MOOCOW_DIR    ?=$(shell pwd)
MOOCOW_BUILD  ?=$(MOOCOW_DIR)/build.moo
MOOCOW_MFLAGS ?=DESTDIR=""

CONFIGURE_ARGS=\
	CFLAGS="$(CFLAGS)" \
	--with-pd-include="$(pd_src)/src" \
	--with-pd-dir="$(MOOCOW_BUILD)" \
	--disable-dependency-tracking

#	--with-pd-extdir="$(MOOCOW_BUILD)/externs"

##-- defaults
CFLAGS ?= -g -O2
pd_src ?= $(CURDIR)/../../../pd


##======================================================================
## Rules: default
all: build.stamp

##======================================================================
## Templates: subdir

## RULES = $(call subdir_template,$(dir_basename),$(dir_path),$(configure_args))
define subdir_template
$(1).autogen_stamp: 
	(cd $(2); sh ./autogen.sh) || true
	touch $$@

$(1).configure_stamp: $(1).autogen_stamp
	(cd $(2); sh ./configure $(CONFIGURE_ARGS) $(3); make clean) || true
	touch $$@

$(1).build_stamp: $(1).configure_stamp
	$(MAKE) $(MOOCOW_MFLAGS) -C $(2) all install || true
	touch $$@

$(1).extclean:
	rm -f $(1).autogen_stamp $(1).configure_stamp $(1).build_stamp

$(1).clean: $(1).extclean
	$(MAKE) -C $(2) clean || true

$(1).distclean: $(1).extclean
	$(MAKE) -C $(2) distclean || true

$(1).cvsclean: $(1).extclean
	$(MAKE) -C $(2) cvsclean || true

endef

##======================================================================
## Rules: subdirectories

##-- flite, gfsm: simulate failed builds
#$(eval $(call subdir_template,flite,../flite,--with-flite-dir=/NOPE))
#$(eval $(call subdir_template,gfsm,../gfsm,--disable-gfsm))

##-- flite, gfsm: build 'em if you got 'em
$(eval $(call subdir_template,flite,../flite,))
$(eval $(call subdir_template,gfsm,../gfsm,))

##-- the usual suspects
$(eval $(call subdir_template,deque,../deque,))
$(eval $(call subdir_template,pdstring,../pdstring,--enable-object-externals))
$(eval $(call subdir_template,readdir,../readdir,))
$(eval $(call subdir_template,sprinkler,../sprinkler,))
$(eval $(call subdir_template,weightmap,../weightmap,))


##======================================================================
## Rules: local
autogen: $(SUBDIRS:=.autogen_stamp)
configure: $(SUBDIRS:=.configure_stamp)

build.stamp: $(SUBDIRS:=.build_stamp)
	touch $@

readme: README.txt
README.txt: README.pod
	pod2text README.pod $@

extclean: $(SUBDIRS:=.extclean)

clean: $(SUBDIRS:=.clean)
	rm  -f build.stamp config.log
	rm -rf $(MOOCOW_BUILD)

realclean: distclean

distclean: $(SUBDIRS:=.distclean) clean

cvsclean: $(SUBDIRS:=.cvsclean) clean
