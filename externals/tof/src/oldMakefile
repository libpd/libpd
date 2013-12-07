current:
	echo make pd_linux, pd_win, or pd_darwin

clean: ; rm -f *.pd_linux *.o

# ----------------------- WINDOWS -----------------------

pd_win: breakpoints.dll breakpoints~.dll destroysend.dll folderpanel.dll getdollarzero.dll phasorshot~.dll

.SUFFIXES: .dll

WINCFLAGS = -Wall -W -Wshadow -Wstrict-prototypes -DPD -DNT -W3 -WX -Wno-unused -mms-bitfields -Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer

WINPDPATH = /d/pd/bin

WININCLUDE =  -I.. -I../include -I$(WINPDPATH)/src

LDFLAGS = -shared

.c.dll:
	gcc -mms-bitfields $(WINCFLAGS) $(WININCLUDE) -o $*.o -c $*.c
	gcc $(LDFLAGS) -o $*.dll $*.o $(WINPDPATH)/bin/pd.dll 	
	strip --strip-unneeded $*.dll
	rm -f $*.o
	


# ----------------------- LINUX i386 -----------------------

pd_linux: breakpoints.pd_linux breakpoints~.pd_linux destroysend.pd_linux folderpanel.pd_linux getdollarzero.pd_linux phasorshot~.pd_linux

.SUFFIXES: .pd_linux

PDPATH = /home/tom/pd/cvs/pd

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I$(PDPATH)/src

.c.pd_linux:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	ld --export-dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o

# ----------------------- Mac OSX -----------------------

pd_darwin: breakpoints~.pd_darwin breakpoints.pd_darwin destroysend.pd_darwin folderpanel.pd_darwin getdollarzero.pd_darwin .pd_darwin

.SUFFIXES: .pd_darwin

DARWINCFLAGS = -DPD -O2 -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

.c.pd_darwin:
	cc $(DARWINCFLAGS) $(LINUXINCLUDE) -o $*.o -c $*.c
	cc -bundle -undefined suppress -flat_namespace -o $*.pd_darwin $*.o 
	rm -f $*.o

