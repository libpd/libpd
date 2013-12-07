default: all

.PHONEY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs \
	tests

HELPERSOURCES=iem_adaptfilt.c
OBJECTSOURCES=$(sort $(filter-out $(HELPERSOURCES), $(filter %.c, $(wildcard *.c))))
SOURCES=$(OBJECTSOURCES) $(HELPERSOURCES)


configure: configure.ac
	autoconf

-include $(SOURCES:.c=.d)

Make.config: Make.config.in configure
	./configure $(CONFIGUREFLAGS)

-include Make.config


##  2nd only generate depend-files when we have Make.config included
##  and thus MAKEDEP_FLAGS defined
ifdef MAKEDEP_FLAGS
## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
%.d: %.c
	@set -e; rm -f $@; \
	 $(CPP) $(MAKEDEP_FLAGS) $(CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
endif

.SUFFIXES: .$(EXT)

TARGETS = $(SOURCES:.c=.o)

OBJECTS = $(OBJECTSOURCES:.c=.$(EXT))


## if $(BUILDLIBRARY) is defined, we build everything as a single library
## else we build separate externals
ifneq "$(BUILDLIBRARY)" ""
all: $(LIBNAME)
	cp $(LIBNAME).$(EXT) ..
else
all: $(OBJECTS)
endif

$(OBJECTS): %.$(EXT) : %.o
	$(LD) $(LFLAGS) -o $@ $*.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $@


$(LIBNAME): $(TARGETS)
	$(LD) $(LFLAGS) -o $@.$(EXT) *.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $(LIBNAME).$(EXT)

$(TARGETS): %.o : %.c
	$(CC) $(CFLAGS) -c -o $@ $*.c

clean:
	-rm -f *.$(EXT) *.o 

realclean: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

distclean: realclean
	-rm -f Make.config
	-rm -f *.exp  *.lib  *.ncb  *.opt  *.plg
	-rm -rf autom4te.cache/

install: install-bin install-doc

install-bin:
	-install -d $(INSTALL_BIN)
	-install -m 644 *.$(EXT) $(INSTALL_BIN)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 *-help.pd $(INSTALL_DOC)

everything: clean all install distclean
