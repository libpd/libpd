default: all

.PHONEY: default all everything dist \
	clean realclean distclean \
	install install-bin install-doc install-abs

SOURCES=$(sort $(filter %.c, $(wildcard *.c)))

ifeq (,$(findstring clean, $(MAKECMDGOALS)))
Make.config: Make.config.in configure
	./configure
endif

iem_bin_ambi_sources.c iem_bin_ambi_sources.h:
	./makesource.sh

configure: configure.ac
	autoconf

## uaehh, here comes some magic
##  1st we don't want depend and config-makefiles to be included on "clean"-targets

ifeq (,$(findstring clean, $(MAKECMDGOALS)))
-include $(SOURCES:.c=.d)
endif

-include Make.config

##  2nd only generate depend-files when we have Make.config included
##  and thus MAKEDEP_FLAGS defined
ifdef MAKEDEP_FLAGS
## dependencies: as proposed by the GNU-make documentation
## see http://www.gnu.org/software/make/manual/html_node/make_47.html#SEC51
%.d: %.c
	@set -e; rm -f $@; \
	 $(CPP) $(MAKEDEP_FLAGS) $(Z_CFLAGS) $< > $@.$$$$; \
	 sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
	 rm -f $@.$$$$
endif

.SUFFIXES: .$(EXT)

TARGETS = $(SOURCES:.c=.o)


all: $(LIBNAME)
	cp $(LIBNAME).$(EXT) ..

$(LIBNAME): $(TARGETS) iem_bin_ambi_sources.c iem_bin_ambi_sources.h
	$(LD) $(LFLAGS) -o $(LIBNAME).$(EXT) *.o $(LIBS)
	$(STRIP) $(STRIPFLAGS) $(LIBNAME).$(EXT)

$(TARGETS): %.o : %.c
	$(CC) $(Z_CFLAGS) -c -o $@ $*.c


clean:
	-rm -f *.$(EXT) *.o 

realclean: clean
	-rm -f *~ _* config.*
	-rm -f *.d *.d.*

distclean: realclean
	-rm -f Make.config ../*.$(EXT)
	-rm -f *.exp  *.lib  *.ncb  \
		*.opt  *.plg
	-rm -rf autom4te.cache/

install: install-bin install-doc install-abs

install-bin:
	-install -d $(INSTALL_BIN)
	-install -m 644 $(LIBNAME).$(EXT) $(INSTALL_BIN)

install-doc:
	-install -d $(INSTALL_DOC)
	-install -m 644 ../examples/*.pd $(INSTALL_DOC)

install-abs:
	-install -d $(INSTALL_BIN)
	-install -m 644 ../abs/*.pd $(INSTALL_BIN)

dist: all realclean
	(cd ../..;tar czvf $(TARNAME) $(LIBNAME))

everything: clean all install distclean

