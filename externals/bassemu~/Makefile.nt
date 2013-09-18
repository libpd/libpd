NAME=bassemu~
CSYM=bassemu

current: pd_linux

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

VC="C:\Programme\Microsoft Platform SDK"
VCC="C:\Programme\Microsoft Visual Studio 8"
PDPATH="C:\Programme\pd"
PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo


PDNTINCLUDE = /I. /I$(PDPATH)\tcl\include /I$(PDPATH)\src /I$(VC)\include /I$(LIBUSBPATH)\include

PDNTLDIR = $(VC)\lib
# --- PDNTLIB = $(PDNTLDIR)\libc.lib \
# --- PDNTLIB = $(PDNTLDIR)\oldnames.lib \
PDNTLIB = $(PDNTLDIR)\uuid.lib \
	$(PDPATH)\bin\pd.lib  \
	$(PDPATH)\bin\pthreadVC.lib

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTLIB)

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -O3 -funroll-loops -Wall -fomit-frame-pointer -fPIC \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch $(CFLAGS)

LINUXINCLUDE =  -I../../src

.c.pd_linux:
	$(CC) $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld --export-dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm -f $*.o

# ----------------------- Mac OSX -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O3 -funroll-loops -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.pd_darwin:
	$(CC) $(DARWINCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	$(CC) -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o
	rm -f $*.o

# ----------------------------------------------------------

clean:
	rm -f *.o *.pd_* so_locations
