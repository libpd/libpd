# Adapt the PDPATH if your pd is not installed in the standard place

PDPATH="$(PROGRAMFILES)/pd"

EXTERNALS = $(shell ls *.c)

all: $(EXTERNALS:.c=.dll)

.SUFFIXES: .dll

DEFINES = -DPD -DNT 
CFLAGS = -Wall -W -Wshadow -Wstrict-prototypes \
    -Wno-unused -Wno-parentheses -Wno-switch

INCLUDE = -I.. -I. -I$(PDPATH)/src
LIBS = -lwsock32

%.dll: %.c
	$(CC) -mms-bitfields $(CFLAGS) $(DEFINES) $(INCLUDE) -o "$*.o" -c "$*.c"
	gcc -shared -o "$*.dll" "$*.o" $(PDPATH)/bin/pd.dll $(LIBS)
	rm "$*.o" 

clean:
	-rm *.a *.def *.dll *.o