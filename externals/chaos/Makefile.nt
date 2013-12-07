NAME=chaos
CSYM=chaos

current: pd_nt pd_intel pd_linux pd_darwin

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /MD /O2 /G6 /DNT /DPD /DMAXLIB /nologo
VC="C:\Programme\Microsoft Visual Studio\VC98"

PDNTINCLUDE = /I. /Ic:\pd\tcl\include /Ic:\pd\src /I$(VC)\include /Iinclude

PDNTLDIR = $(VC)\Lib
PDNTLIB = $(PDNTLDIR)\msvcrt.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\user32.lib \
	$(PDNTLDIR)\uuid.lib \
	$(PDNTLDIR)\ws2_32.lib \
	$(PDNTLDIR)\pthreadVC.lib \
	c:\pd\bin\pd.lib
        
PDNTEXTERNALS = lyapunov.obj henon.obj hopalong.obj ikeda.obj latoocarfian.obj latoomutalpha.obj latoomutbeta.obj latoomutgamma.obj logistic.obj lorenz.obj mlogistic.obj rossler.obj standardmap.obj tent.obj three_d.obj

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c *.c
	link /dll /export:$(CSYM)_setup $*.obj $(PDNTEXTERNALS) $(PDNTLIB)

# ----------------------- Mac OS X (Darwin) -----------------------

pd_darwin: $(NAME).pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -DMAXLIB -DUNIX -DMACOSX -O2 \
    -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
DARWININCLUDE =  -I../../src -I../../obj

DARWINEXTERNALS = lyapunov.o attract1.o base.o base3.o dejong.o gingerbreadman.o henon.o hopalong.o ikeda.o latoocarfian.o latoomutalpha.o \
latoomutbeta.o latoomutgamma.o logistic.o lorenz.o martin.o mlogistic.o pickover.o popcorn.o quadruptwo.o rossler.o standardmap.o \
strange1.o tent.o three_d.o threeply.o tinkerbell.o unity.o lotka_volterra.o

PD_LOCAL_EXTERNALS = $(HOME)/Library/Pd/Externals
PD_LOCAL_HELP = $(HOME)/Library/Pd/Help
PD_BINARY = /usr/local/pd/bin/pd
MTARGET = $(NAME).pd_darwin
MDARWININCLUDE =  -I/usr/local/pd/src

.c.o:
	cc $(DARWINCFLAGS) $(DARWININCLUDE) -c *.c

.o.pd_darwin:
	cc -bundle -bundle_loader $(PD_BINARY) -flat_namespace -o $*.pd_darwin $*.o $(DARWINEXTERNALS)

pd_darwin: $(DARWINEXTERNALS)
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

minstall: $(MTARGET) $(DARWINEXTERNALS)
	mv -f $(MTARGET) $(PD_LOCAL_EXTERNALS)
	cp -f help-*.pd $(PD_LOCAL_HELP)

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow \
    -Wno-unused -Wno-parentheses -Wno-switch

# where is your m_pd.h ???
LINUXINCLUDE =  -I/usr/local/include 

LINUXEXTERNALS = lyapunov.o attract1.o base.o base3.o dejong.o gingerbreadman.o henon.o hopalong.o ikeda.o latoocarfian.o latoomutalpha.o \
latoomutbeta.o latoomutgamma.o logistic.o lorenz.o martin.o mlogistic.o pickover.o popcorn.o quadruptwo.o rossler.o standardmap.o \
strange1.o tent.o three_d.o threeply.o tinkerbell.o unity.o lotka_volterra.o

.c.pd_linux:
	cc -O2 -Wall -DPD -fPIC $(LINUXCFLAGS) $(LINUXINCLUDE) -c *.c 
	ld --export-dynamic  -shared -o $*.pd_linux $*.o $(LINUXEXTERNALS) -lc 
	strip --strip-unneeded $*.pd_linux 

# ----------------------------------------------------------

PDDIR=/usr/lib/pd

clean:
	rm -f *.o *.pd_* so_locations
