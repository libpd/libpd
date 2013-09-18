default: all

-include Make.config

.PHONY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs \
	tests \
	$(LIBRAY_NAME) library externals

VPATH = .:../alias/
.SUFFIXES: .$(EXT)


TESTDIR=../tests


#####################
# CORE: core library, ought to be statically-linked with each and every .pd_linux
# LHELPER: infrastructure for iemmatrix.pd_linux
# ????: 
# ALIAS: alias-objects, MUST NOT be linked statically into iemmatrix.pd_linux

CORESRC=$(LIBRARY_NAME)_binops.c $(LIBRARY_NAME)_utility.c
LHELPERSRC=$(LIBRARY_NAME).c $(LIBRARY_NAME)_sources.c
PDOBJECTSRC=$(sort $(filter-out $(LHELPERSRC) $(CORESRC), $(filter %.c, $(wildcard *.c))))
ALIASSRC=$(subst ../alias/,,$(sort $(filter %.c, $(wildcard ../alias/*.c))))

ALLSRC=$(CORESRC) $(LHELPERSRC) $(PDOBJECTSRC) $(ALIASSRC)

COREOBJ=$(CORESRC:.c=.o)
LHELPEROBJ=$(LHELPERSRC:.c=.o)
PDOBJ=$(PDOBJECTSRC:.c=.o)
ALIASOBJ=$(ALIASSRC:.c=.o)

PDOBJECTS=$(PDOBJECTSRC:.c=.$(EXT))
ALIASOBJECTS=$(ALIASSRC:.c=.$(EXT))


-include $(ALLSRC:.c=.d)

##  2nd only generate depend-files when we have Make.config included
##  and thus MAKEDEP_FLAGS defined
ifdef MAKEDEP_FLAGS
## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
%.d: %.c
	@set -e; rm -f $@; \
	 $(CPP) $(MAKEDEP_FLAGS) $(LIBRARY_CPPFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
endif


## if $(BUILDLIBRARY) is defined, the default is to
## build everything as a single library
## else we build separate externals
ifneq "$(BUILDLIBRARY)" ""
all: $(LIBRARY_NAME)
	cp $(LIBRARY_NAME).$(EXT) ..
else
all: externals
endif

%.o : %.c
	$(CC) $(LIBRARY_CFLAGS) $(AFLAGS) -c -o $@ $<

%.$(EXT) : %.o $(COREOBJ)
	$(LD) $(LFLAGS) $(AFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@

$(LIBRARY_NAME).$(EXT): $(COREOBJ) $(LHELPEROBJ) $(PDOBJ) 
	$(LD) $(LFLAGS) $(AFLAGS) -o $@ $^ $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@

$(LIBRARY_NAME): $(LIBRARY_NAME).$(EXT)
	@echo "$< done"

externals: $(PDOBJECTS) $(ALIASOBJECTS)

library: $(LIBRARY_NAME)


clean:
	-rm -f *.$(EXT) *.o 

METAFILE = ../iemmatrix-meta.pd

realclean: clean
	-rm -f *~ _* aclocal.m4 configure config.h config.log config.status
	-rm -f *.d *.d.*
	-rm -f ../VERSION.txt $(METAFILE)

distclean: realclean
	-rm -f Make.config ../*.$(EXT)
	-rm -f $(LIBRARY_NAME).exp  $(LIBRARY_NAME).lib  $(LIBRARY_NAME).ncb  \
		$(LIBRARY_NAME).opt  $(LIBRARY_NAME).plg
	-rm -rf autom4te.cache/

tests:  all
	make -C $(TESTDIR)

install: install-bin install-doc install-abs
	-install -d $(INSTALL_DOC)
	-install -m 644 ../VERSION.txt $(INSTALL_DOC)
	-install -m 644 $(METAFILE) $(INSTALL_DOC)

install-bin:
	-install -d $(INSTALL_DOC)
	-install -m 644 *.$(EXT) $(INSTALL_DOC)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../doc/*.pd $(INSTALL_DOC)

install-abs:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../abs/*.pd $(INSTALL_DOC)

dist: all realclean
	(cd ../..;tar czvf $(TARNAME) $(LIBRARY_NAME))

everything: clean all install distclean


#######################
configure: configure.ac aclocal.m4
	autoconf

aclocal.m4: acinclude.m4
	aclocal

Make.config: Make.config.in configure
	./configure $(CONFIGUREFLAGS)

$(LIBRARY_NAME)_sources.c $(LIBRARY_NAME)_sources.h:
	./makesource.sh
#######################



debug:
	@echo SOURCES $(SOURCES)
	@echo HELPERSOURCES $(HELPERSOURCES)
	@echo ALIASSOURCES $(ALIASOBJECTSOURCES)
	@echo TARGETS $(TARGETS)
	@echo OBJECTS $(PDOBJECTS)
