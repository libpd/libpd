current: all

.SUFFIXES: .pd_linux

# make sure that the "m_pd.h" is somehow available either by putting it into this
# directory, by adding it's path to the INCLUDE-path or by putting it into an
# already included path, e.g. "/usr/include/"
INCLUDE = -I. -I/usr/include/

PDPATH = /usr/lib/pd

LDFLAGS = -export-dynamic -shared

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer

SYSTEM = $(shell uname -m)

# the sources:

SRC = adaptive.c lms~.c lms2~.c nlms~.c nlms2~.c nlms3~.c

TARGET = adaptive.pd_linux


OBJ = $(SRC:.c=.o)


#  ------------------ targets ------------------------------------


clean:
	rm -f *.o *.pd_linux

all: $(OBJ)
	@echo :: $(OBJ)
	ld $(LDFLAGS) -o $(TARGET) $(OBJ)
	strip --strip-unneeded $(TARGET)

$(OBJ) : %.o : %.c
	touch $*.c
	cc $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c

install:
	cp $(TARGET) $(PDPATH)/externs
	cp ../doc/help-*.pd $(PDPATH)/doc/5.reference
