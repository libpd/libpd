default: 

PARAM: param paramDump paramRoute paramFile paramId paramCustom

current:
	echo make class

clean: 
	rm -f *.pd_linux *.o

# ----------------------- LINUX i386 -----------------------

#paramL:  param.pd_linux

#patchArguments: patchArguments.pd_linux

.SUFFIXES: .pd_linux

PDPATH = ../../../pd

TOFPATH = ../src

LINUXCFLAGS = -DPD -O2 -funroll-loops -fomit-frame-pointer \
    -Wall -W -Wshadow -Wstrict-prototypes  \
    -Wno-unused -Wno-parentheses -Wno-switch

LINUXINCLUDE =  -I$(PDPATH)/src

#.pd_linux: .c

.c:
	cc $(LINUXCFLAGS) $(LINUXINCLUDE) -I $(TOFPATH) -o $*.o -c $*.c
	ld --export-dynamic  -shared -o $*.pd_linux $*.o -lc -lm
	strip --strip-unneeded $*.pd_linux
	rm $*.o
	mv $*.pd_linux ~/pd-externals/tof
	cp $*-help.pd ~/pd-externals/tof
