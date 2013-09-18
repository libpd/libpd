NAME=pdogg
CSYM=pdogg

current: pd_nt pd_darwin pd_linux

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /MD /O2 /G6 /DNT /DPD /nologo
VC = "C:\Programme\Microsoft Visual Studio\VC98"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\msvcrt.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	$(PDNTLDIR)\uuid.lib \
	$(PDNTLDIR)\ws2_32.lib \
	$(PDNTLDIR)\pthreadVC.lib \
	lib\ogg_static.lib \
	lib\vorbis_static.lib \
	lib\vorbisenc_static.lib \
	lib\vorbisfile_static.lib \
	c:\pd\bin\pd.lib
        

EXTERNALS = oggamp~.obj oggcast~.obj oggread~.obj oggwrite~.obj

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c oggamp~\oggamp~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c oggcast~\oggcast~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c oggread~\oggread~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c oggwrite~\oggwrite~.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /NODEFAULTLIB:libcmt.lib /NODEFAULTLIB:libc.lib /export:$(CSYM)_setup $*.obj \
	$(EXTERNALS) $(PDNTLIB)

# ----------------------- Mac OS X -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DUNIX -DMACOSX -O3 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../src -I. -Iinclude

DARWINEXTERNALS = oggamp~.o oggcast~.o oggread~.o oggwrite~.o

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c oggamp~/oggamp~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c oggcast~/oggcast~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c oggread~/oggread~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c oggwrite~/oggwrite~.c
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c $*.c 
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS) \
	-Llib -lvorbisfile -lvorbisenc -lvorbis -logg
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I../../src -I ../../pd/src

LINUXEXTERNALS = oggamp~.o oggcast~.o oggread~.o oggwrite~.o

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c oggamp~/oggamp~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c oggcast~/oggcast~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c oggread~/oggread~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c oggwrite~/oggwrite~.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -c $*.c
	ld --export-dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc \
           -lm -L/usr/local/lib -lvorbisfile -lvorbisenc -lvorbis -logg
	strip --strip-unneeded $*.pd_linux

# ----------------------------------------------------------

PDDIR=/usr/lib/pd

install:
	install -d $(PDDIR)/doc/5.reference/pdogg
	cp *-help.pd ../../doc/5.reference/pdogg

clean:
	rm -f *.o *.pd_* so_locations
