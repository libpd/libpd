## File: config/Makefile.am
## Package:
## Description:
##   + automake file for 'config' package-subdir
##
## Process this file with Automake to create Makefile.in.
##-----------------------------------------------------------------------

##-----------------------------------------------------------------------
## Variables: options
##-----------------------------------------------------------------------

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
	*~ .*~ \
	compile Makefile Makefile.in \
	config.guess \
	config.sub \
	depcomp \
	install-sh \
	ltmain.sh \
	missing \
	texinfo.tex \
	mkinstalldirs \
	ylwrap



##-----------------------------------------------------------------------
## Variables: distribution
##-----------------------------------------------------------------------

## --- extra distribution files
EXTRA_DIST = \
	Makefile.in \
	depcomp \
	install-sh \
	$(wildcard mkinstalldirs) \
	missing

#	mkinstalldirs
#	config.guess
#	config.sub
#	ltmain.sh
#	texinfo.tex

## --- recursion subdirectories for 'make dist'
#DIST_SUBDIRS = $(SUBDIRS)

#-----------------------------------------------------------------------
# Rules: cleanup
#-----------------------------------------------------------------------
.PHONY: cvsclean cvsclean-hook

cvsclean: maintainer-clean ;
