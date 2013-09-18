default: all

.PHONEY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs \
	tests

-include Make.config

TESTDIR=../tests

HELPERSOURCES=$(LIBRARY_NAME).c

OBJECTSOURCES=$(sort $(filter-out $(HELPERSOURCES), $(filter %.c, $(wildcard *.c))))

SOURCES=$(OBJECTSOURCES) $(HELPERSOURCES)

configure: configure.ac aclocal.m4
	autoconf

aclocal.m4: acinclude.m4
	aclocal

Make.config: Make.config.in configure
	./configure $(CONFIGUREFLAGS)

-include $(SOURCES:.c=.d)

##  2nd only generate depend-files when we have Make.config included
##  and thus MAKEDEP_FLAGS defined
ifdef MAKEDEP_FLAGS
## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
%.d: %.c
	@set -e; rm -f $@; \
	 $(CPP) $(MAKEDEP_FLAGS) $(LIBRARY_CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
endif

.SUFFIXES: .$(EXT)

TARGETS = $(SOURCES:.c=.o)

OBJECTS = $(OBJECTSOURCES:.c=.$(EXT))

## if $(BUILDLIBRARY) is defined, we build everything as a single library
## else we build separate externals
ifneq "$(BUILDLIBRARY)" ""
all: $(LIBRARY_NAME)
	cp $(LIBRARY_NAME).$(EXT) ..
else
all: $(OBJECTS)
endif

$(OBJECTS): %.$(EXT) : %.o
	$(LD) $(LFLAGS) -o $@ $*.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@

$(LIBRARY_NAME): $(TARGETS)
	$(LD) $(LFLAGS) -o $@.$(EXT) *.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@.$(EXT)

$(TARGETS): %.o : %.c
	$(CC) $(LIBRARY_CFLAGS) -c -o $@ $*.c

externals: $(OBJECTS)

clean:
	-rm -f *.$(EXT) *.o 

realclean: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

distclean: realclean
	-rm -f Make.config ../*.$(EXT)
	-rm -f $(LIBRARY_NAME).exp  $(LIBRARY_NAME).lib  $(LIBRARY_NAME).ncb  \
		$(LIBRARY_NAME).opt  $(LIBRARY_NAME).plg
	-rm -rf autom4te.cache/

tests:  all
	make -C $(TESTDIR)

install: install-bin install-doc install-abs

install-bin:
	-install -d $(INSTALL_BIN)
	-install -m 644 $(LIBRARY_NAME).$(EXT) $(INSTALL_BIN)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../doc/*.pd $(INSTALL_DOC)

install-abs:
	-install -d $(INSTALL_BIN)
	-install -m 644 ../abs/*.pd $(INSTALL_BIN)

dist: all realclean
	(cd ../..;tar czvf $(TARNAME) $(LIBRARY_NAME))

everything: clean all install distclean

