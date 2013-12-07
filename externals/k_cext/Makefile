NAME=k_cext
CSYM=k_cext

DIR=k_cext

current: pd_linux

# ----------------------- NT -----------------------

pd_nt: $(NAME).dll

.SUFFIXES: .dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo /DINCLUDEPATH=\"D:\\sourcescvs\\pd\"

PDNTINCLUDE = /I. /ID:\sourcescvs\pd\src
PDNTLIB = libc.lib oldnames.lib kernel32.lib "C:\Pure Data\bin\pd.lib"

.c.dll:
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c
	cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c k_cext_win.c
	link /dll /export:$(CSYM)_setup $*.obj k_cext_win.obj $(PDNTLIB)

# ----------------------- IRIX 5.x -----------------------

pd_irix5: $(NAME).pd_irix5

.SUFFIXES: .pd_irix5

SGICFLAGS5 = -o32 -DPD -DUNIX -DIRIX -O2

SGIINCLUDE =  -I../../src

.c.pd_irix5:
	cc $(SGICFLAGS5) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -elf -shared -rdata_shared -o $*.pd_irix5 $*.o
	rm $*.o

# ----------------------- IRIX 6.x -----------------------

pd_irix6: $(NAME).pd_irix6

.SUFFIXES: .pd_irix6

SGICFLAGS6 = -n32 -DPD -DUNIX -DIRIX -DN32 -woff 1080,1064,1185 \
	-OPT:roundoff=3 -OPT:IEEE_arithmetic=3 -OPT:cray_ivdep=true \
	-Ofast=ip32

.c.pd_irix6:
	cc $(SGICFLAGS6) $(SGIINCLUDE) -o $*.o -c $*.c
	ld -n32 -IPA -shared -rdata_shared -o $*.pd_irix6 $*.o
	rm $*.o

# ----------------------- LINUX i386 -----------------------

pd_linux: $(NAME).pd_linux  k_cext.c k_cext.h  k_cext_generatecode.c

.SUFFIXES: .pd_linux

LINUXCFLAGS = -DPD -DUNIX -DICECAST -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wno-shadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch -fPIC #-Werror

LINUXINCLUDEPATH=../../src
LINUXINCLUDE =  -I$(LINUXINCLUDEPATH)

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -DINCLUDEPATH=\""`pwd`"\" -DLINUXINCLUDE=\""$(LINUXINCLUDEPATH)"\" -o k_cext.o -c k_cext.c
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -DINCLUDEPATH=\""`pwd`"\" -DLINUXINCLUDE=\""$(LINUXINCLUDEPATH)"\" -o k_cext_unix.o -c k_cext_unix.c
	cc -shared -o k_cext.pd_linux k_cext.o k_cext_unix.o -lc -lm -fPIC
	strip --strip-unneeded $*.pd_linux
	rm -f $*.o ../$*.pd_linux
	ln -s $(DIR)/$*.pd_linux ..
	ln -sf $(DIR)/$*.pd_linux ../k_cfunc.pd_linux

# ----------------------- Mac OSX -----------------------

pd_darwin: $(NAME).pd_darwin k_cext.c k_cext.h   k_cext_generatecode.c

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O2 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(LINUXINCLUDE) -DINCLUDEPATH=\""`pwd`"\" -DLINUXINCLUDE=\""$(LINUXINCLUDEPATH)"\ -o $*.o -c k_cext.c
	cc $(DARWINCFLAGS) $(LINUXINCLUDE) -DINCLUDEPATH=\""`pwd`"\" -DLINUXINCLUDE=\""$(LINUXINCLUDEPATH)"\ -o $*.o -c k_cext_macosx.c
	cc -bundle -undefined suppress  -flat_namespace -o $*.pd_darwin $*.o 
	rm -f $*.o ../$*.pd_darwin
	ln -s $*/$*.pd_darwin ..

# ----------------------------------------------------------

install:
	cp help-*.pd ../../doc/5.reference

clean:
	rm -f *.o *.pd_* so_locations *~ core
