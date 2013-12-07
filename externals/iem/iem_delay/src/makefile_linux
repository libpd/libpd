current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Werror -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer -fno-strict-aliasing \
        -DDL_OPEN -fPIC

SYSTEM = $(shell uname -m)

# the sources

SRC = n_delay1p_line~.c \
	n_delay2p_line~.c \
	nz~.c \
	block_delay~.c \
	iem_delay.c

TARGET = iem_delay.pd_linux


OBJ = $(SRC:.c=.o) 

#
#  ------------------ targets ------------------------------------
#

clean:
	rm ../$(TARGET)
	rm *.o

all: $(OBJ)
	@echo :: $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET) *.o $(LIB)
	mv $(TARGET) ..
	strip --strip-unneeded $(TARGET)

$(OBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c




