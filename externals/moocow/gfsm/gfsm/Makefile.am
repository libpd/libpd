# File: ./Makefile.am
# Package: gfsm
# Description:
#   + top-level automake file
#
# Process this file with Automake to create Makefile.in.
#-----------------------------------------------------------------------

#-----------------------------------------------------------------------
# Options & Subdirectories
#-----------------------------------------------------------------------

## --- automake options
#AUTOMAKE_OPTIONS = foreign dist-bzip2 dist-zip
AUTOMAKE_OPTIONS = foreign

## --- recursion subdirectories
#SUBDIRS = config src doc tests
SUBDIRS = config src

## --- pseudo-deps for '.SUFFIXES'
SUFFIXES = .pc .pod .txt .html

## --- package config data
#pkgconfig_DATA = @PACKAGE@.pc

##-- for gnulib
ACLOCAL_AMFLAGS = -I m4

#-----------------------------------------------------------------------
# Variables: cleanup
#-----------------------------------------------------------------------
## --- mostlyclean: built by 'make' & commonly rebuilt
#MOSTLYCLEANFILES =

## --- clean: built by 'make'
#CLEANFILES =

## --- distclean: built by 'configure'
DISTCLEANFILES = \
	config.log	\
	config.cache	\
	config.status

## -- maintainerclean: built by maintainer / by hand
MAINTAINERCLEANFILES = *~ \
	Makefile Makefile.in \
	aclocal.m4 \
	configure \
	install-sh \
	stamp-h.in \
	config.h.in \
	magic.local \
	magic \
	*.tmp

#	$(PODS:.pod=.txt)
#	$(PODS:.pod=.html)

maintainer-clean-local:
	rm -rf autom4te.cache

#CVSCLEAN_SUBDIRS = $(SUBDIRS)

#CVSCLEANFILES = Makefile.in Makefile


#-----------------------------------------------------------------------
# Additional Variables & Rules: PODS
#-----------------------------------------------------------------------
PODS = README.pod

if HAVE_POD2TEXT
.pod.txt:
	pod2text $< $@
endif

if HAVE_POD2HTML
.pod.html:
	pod2html --outfile $@ $<
endif

#pkgdoc_DATA = \
#	$(PODS:.pod=.txt) \
#	$(PODS:.pod=.html)

#-----------------------------------------------------------------------
# Additional Variables & Rules: magic
#-----------------------------------------------------------------------

#pkgdoc_DATA += gfsm.magic

##-- phony
.PHONY: install-magic unintall-magic

MAGIC_ADDRS='/^\#\#--BEGIN gfsm\.magic$$/,/^\#\#--END gfsm\.magic/'
magic     ?=/etc/magic

install-magic: magic
	$(INSTALL_DATA) $< $(magic)

uninstall-magic: magic.local
	$(INSTALL_DATA) $< $(magic)

magic: gfsm.magic magic.local
	cat magic.local > "$@"
	sed -n -e$(MAGIC_ADDRS)p @srcdir@/gfsm.magic >> "$@"

magic.local: $(ETC_MAGIC)
	sed -e$(MAGIC_ADDRS)d $(magic) > $@


#-----------------------------------------------------------------------
# Variables: distribution
#-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	INSTALL \
	AUTHORS \
	Changes \
	$(PODS) \
	$(PODS:.pod=.txt) \
	$(PODS:.pod=.html) \
	autogen.sh \
	configure \
	README.cvs \
	gfsm.magic \
	configure.gnu

#COPYING

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

#dist-bz2: dist-bzip2 ;


#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;

