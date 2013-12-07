# To use this Makefile for your project, first put the name of your library in
# LIBRARY_NAME variable. The folder for your project should have the same name
# as your library.
LIBRARY_NAME = extra

# Next, add your source files to the SOURCES variable.
SOURCES = bonk~/bonk~.c choice/choice.c fiddle~/fiddle~.c loop~/loop~.c lrshift~/lrshift~.c pique/pique.c sigmund~/sigmund~.c stdout/stdout.c

# For objects that only build on certain platforms, add those to the SOURCES
# line for the right platforms.
SOURCES_Darwin = pd~/pd~.c
SOURCES_Linux = pd~/pd~.c

# include these abstractions with their help patches  
PD_OBJECTS = complex-mod~.pd hilbert~.pd rev1~.pd rev2~.pd rev3~.pd

# these are meant to be customizable
CFLAGS = -Wall -W -g
LDFLAGS = 

#------------------------------------------------------------------------------#
#
# you shouldn't need to edit anything below here, if we did it right :)
#
#------------------------------------------------------------------------------#

# where Pd lives
PD_PATH = ../../pd
# where to install the library
prefix = /usr/local
libdir = $(prefix)/lib
pkglibdir = $(libdir)/pd-externals
objectsdir = $(pkglibdir)

ALL_CFLAGS = -DPD -I$(PD_PATH)/src
ALL_LDFLAGS =  
LIBS = 

LN_S = ln -s

INSTALL = install
INSTALL_PROGRAM = $(INSTALL) -p -m 644
INSTALL_DATA = $(INSTALL) -p -m 644
INSTALL_DIR     = $(INSTALL) -p -m 755 -d

UNAME := $(shell uname -s)
ifeq ($(UNAME),Darwin)
  SOURCES += $(SOURCES_Darwin)
  EXTENSION = pd_darwin
  OPT_CFLAGS = -ftree-vectorize -ftree-vectorizer-verbose=2 -fast
  FAT_FLAGS = -arch i386 -arch ppc -mmacosx-version-min=10.4
  ALL_CFLAGS += -fPIC $(FAT_FLAGS)
  ALL_LDFLAGS += -bundle -undefined dynamic_lookup $(FAT_FLAGS)
  LIBS += -lc -lm
  STRIP = strip -x
 endif
ifeq ($(UNAME),Linux)
  SOURCES += $(SOURCES_Linux)
  EXTENSION = pd_linux
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq ($(UNAME),GNU)
  # GNU/Hurd, should work like GNU/Linux for basically all externals
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq ($(UNAME),GNU/kFreeBSD)
  # Debian GNU/kFreeBSD, should work like GNU/Linux for basically all externals
  SOURCES += $(SOURCES_linux)
  EXTENSION = pd_linux
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq (CYGWIN,$(findstring CYGWIN,$(UNAME)))
#  SOURCES += $(SOURCES_Linux)
  EXTENSION = dll
  OPT_CFLAGS = -O6 -funroll-loops -fomit-frame-pointer
  ALL_CFLAGS += -fPIC
  ALL_LDFLAGS += -L$(PD_PATH)/src -Wl,--export-dynamic  -shared -fPIC
  LIBS += -lc -lpd
  STRIP = strip --strip-unneeded -R .note -R .comment
endif
ifeq (MINGW,$(findstring MINGW,$(UNAME)))
  SOURCES += $(SOURCES_Windows)
  EXTENSION = dll
  CC=gcc
  OPT_CFLAGS = -O3 -funroll-loops -fomit-frame-pointer -march=i686 -mtune=pentium4
  WINDOWS_HACKS = -D'O_NONBLOCK=1'
  ALL_CFLAGS += -mms-bitfields $(WINDOWS_HACKS)
  ALL_LDFLAGS += -s -shared -Wl,--enable-auto-import
  LIBS += -L$(PD_PATH)/src -lpd -lwsock32 -lkernel32 -luser32 -lgdi32
  STRIP = strip --strip-unneeded -R .note -R .comment
endif

ALL_CFLAGS += $(OPT_CFLAGS)


.PHONY = install libdir_install single_install install-doc install-exec clean dist etags

# kludges for objects with strange build requirements
EXPR_OBJECTS = expr~/expr~.$(EXTENSION) fexpr~.$(EXTENSION) expr.$(EXTENSION)
# special cases, where there are multiple .c files for a single class
EXPR_SOURCES = expr~/vexp.c expr~/vexp_fun.c expr~/vexp_if.c


# get library version from meta file
LIBRARY_VERSION = $(shell sed -n 's|^\#X text [0-9][0-9]* [0-9][0-9]* VERSION \(.*\);|\1|p' $(LIBRARY_NAME)-meta.pd)
DISTDIR=$(LIBRARY_NAME)-$(LIBRARY_VERSION)
ORIGDIR=pd-$(LIBRARY_NAME:~=)_$(LIBRARY_VERSION)


all: $(SOURCES:.c=.$(EXTENSION)) $(EXPR_OBJECTS) pd~/pdsched.$(EXTENSION)

%.o: %.c
	$(CC) $(ALL_CFLAGS) $(CFLAGS) -o "$*.o" -c "$*.c"

%.$(EXTENSION): %.o
	$(CC) $(ALL_LDFLAGS) $(LDFLAGS) -o "$*.$(EXTENSION)" "$*.o"  $(LIBS)
	chmod a-x "$*.$(EXTENSION)"


expr~/expr~.$(EXTENSION): $(EXPR_SOURCES:.c=.o)
	$(CC) $(ALL_LDFLAGS) $(LDFLAGS) -o expr~/expr~.$(EXTENSION) $(EXPR_SOURCES:.c=.o) $(LIBS)
	chmod a-x expr~/expr~.$(EXTENSION)

expr.$(EXTENSION): expr~/expr~.$(EXTENSION)
	test -e expr.$(EXTENSION) || \
		$(LN_S) expr~/expr~.$(EXTENSION) expr.$(EXTENSION)

fexpr~.$(EXTENSION): expr~/expr~.$(EXTENSION)
	test -e fexpr~.$(EXTENSION) || \
		$(LN_S) expr~/expr~.$(EXTENSION) fexpr~.$(EXTENSION)

install: all install-doc install-exec install-special

# install all the weird old kludgey build stuff here
install-special:
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) extra-meta.pd $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(SOURCES:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(EXPR_OBJECTS) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(SOURCES:.c=-help.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) rev1-final.pd rev1-stage.pd $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) expr-help.pd $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(STRIP) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/*.$(EXTENSION)

install-doc:
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(SOURCES:.c=-help.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(PD_OBJECTS:.pd=-help.pd) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) pd~/pd~-subprocess.pd pd~/z.pd $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) README.txt $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)/README.txt

install-exec:
	$(INSTALL_DIR) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(SOURCES:.c=.$(EXTENSION)) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) $(PD_OBJECTS) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)
	$(INSTALL_DATA) pd~/pdsched.$(EXTENSION) $(DESTDIR)$(objectsdir)/$(LIBRARY_NAME)

dist:
	$(INSTALL_DIR) $(DISTDIR)
	$(INSTALL_DATA) Makefile $(DISTDIR)
	$(INSTALL_DATA) README.txt $(DISTDIR)
	$(INSTALL_DATA) LICENSE.txt $(DISTDIR)
	$(INSTALL_DATA) extra-meta.pd $(DISTDIR)
	$(INSTALL_DATA) complex-mod~-help.pd $(DISTDIR)
	$(INSTALL_DATA) complex-mod~.pd $(DISTDIR)
	$(INSTALL_DATA) hilbert~-help.pd $(DISTDIR)
	$(INSTALL_DATA) hilbert~.pd $(DISTDIR)
	$(INSTALL_DATA) rev1-final.pd $(DISTDIR)
	$(INSTALL_DATA) rev1~-help.pd $(DISTDIR)
	$(INSTALL_DATA) rev1~.pd $(DISTDIR)
	$(INSTALL_DATA) rev1-stage.pd $(DISTDIR)
	$(INSTALL_DATA) rev2~-help.pd $(DISTDIR)
	$(INSTALL_DATA) rev2~.pd $(DISTDIR)
	$(INSTALL_DATA) rev3~-help.pd $(DISTDIR)
	$(INSTALL_DATA) rev3~.pd $(DISTDIR)
	$(INSTALL_DATA) expr-help.pd $(DISTDIR)
	$(INSTALL_DIR) $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/fts_to_pd.h $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/LICENSE.txt $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/README.txt $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/vexp.c $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/vexp_fun.c $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/vexp.h $(DISTDIR)/expr~
	$(INSTALL_DATA) expr~/vexp_if.c $(DISTDIR)/expr~
	$(INSTALL_DIR) $(DISTDIR)/bonk~
	$(INSTALL_DATA) bonk~/bonk~.c $(DISTDIR)/bonk~
	$(INSTALL_DATA) bonk~/bonk~-help.pd $(DISTDIR)/bonk~
	$(INSTALL_DATA) bonk~/templates.txt $(DISTDIR)/bonk~
	$(INSTALL_DIR) $(DISTDIR)/choice
	$(INSTALL_DATA) choice/choice.c $(DISTDIR)/choice
	$(INSTALL_DATA) choice/choice-help.pd $(DISTDIR)/choice
	$(INSTALL_DIR) $(DISTDIR)/fiddle~
	$(INSTALL_DATA) fiddle~/fiddle~.c $(DISTDIR)/fiddle~
	$(INSTALL_DATA) fiddle~/fiddle~-help.pd $(DISTDIR)/fiddle~
	$(INSTALL_DIR) $(DISTDIR)/loop~
	$(INSTALL_DATA) loop~/loop~.c $(DISTDIR)/loop~
	$(INSTALL_DATA) loop~/loop~-help.pd $(DISTDIR)/loop~
	$(INSTALL_DATA) loop~/test-loop~.pd $(DISTDIR)/loop~
	$(INSTALL_DIR) $(DISTDIR)/lrshift~
	$(INSTALL_DATA) lrshift~/lrshift~.c $(DISTDIR)/lrshift~
	$(INSTALL_DATA) lrshift~/lrshift~-help.pd $(DISTDIR)/lrshift~
	$(INSTALL_DIR) $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/notes.txt $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/pd~.c $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/pd~-help.pd $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/pdsched.c $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/pd~-subprocess.pd $(DISTDIR)/pd~
	$(INSTALL_DATA) pd~/z.pd $(DISTDIR)/pd~
	$(INSTALL_DIR) $(DISTDIR)/pique
	$(INSTALL_DATA) pique/pique.c $(DISTDIR)/pique
	$(INSTALL_DATA) pique/pique-help.pd $(DISTDIR)/pique
	$(INSTALL_DIR) $(DISTDIR)/sigmund~
	$(INSTALL_DATA) sigmund~/sigmund~.c $(DISTDIR)/sigmund~
	$(INSTALL_DATA) sigmund~/sigmund~-help.pd $(DISTDIR)/sigmund~
	$(INSTALL_DIR) $(DISTDIR)/stdout
	$(INSTALL_DATA) stdout/stdout.c $(DISTDIR)/stdout
	$(INSTALL_DATA) stdout/stdout-help.pd $(DISTDIR)/stdout
	tar --exclude-vcs -czpf $(DISTDIR).tar.gz $(DISTDIR)

distclean: clean
	-rm -f -- $(DISTBINDIR).tar.gz
	-rm -rf -- $(DISTBINDIR)
	-rm -f -- $(DISTDIR).tar.gz
	-rm -rf -- $(DISTDIR)
	-rm -f -- $(ORIGDIR).tar.gz
	-rm -rf -- $(ORIGDIR)

# make a Debian source package
dpkg-source:
	debclean
	make distclean dist
	mv $(DISTDIR) $(ORIGDIR)
	tar --exclude-vcs -czpf ../$(ORIGDIR).orig.tar.gz $(ORIGDIR)
	rm -f -- $(DISTDIR).tar.gz
	rm -rf -- $(DISTDIR) $(ORIGDIR)
	cd .. && dpkg-source -b $(LIBRARY_NAME)

clean:
	-rm -f -- $(SOURCES:.c=.o)
	-rm -f -- $(SOURCES:.c=.$(EXTENSION))
	-rm -f -- $(LIBRARY_NAME).$(EXTENSION)
	-rm -f -- $(EXPR_SOURCES:.c=.o)
	-rm -f -- $(EXPR_OBJECTS) pd~/pdsched.$(EXTENSION)

showpaths:
	@echo "PD_PATH: $(PD_PATH)"
	@echo "objectsdir: $(objectsdir)"
	@echo "LIBRARY_NAME: $(LIBRARY_NAME)"
	@echo "SOURCES: $(SOURCES)"
