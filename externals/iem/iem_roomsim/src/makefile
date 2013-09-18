current: all

.SUFFIXES: .pd_linux

INCLUDE = -I. -I/usr/local/src/pd/src

LDFLAGS = -export-dynamic -shared
LIB = -ldl -lm -lpthread

#select either the DBG and OPT compiler flags below:

CFLAGS = -DPD -DUNIX -W -Wno-unused \
	-Wno-parentheses -Wno-switch -O6 -funroll-loops -fomit-frame-pointer -fno-strict-aliasing \
        -DDL_OPEN -fPIC

SYSTEM = $(shell uname -m)

# the sources

SRC = early_reflections_3d.c \
	early_reflections_2d.c \
	cart2del_damp_2d.c \
	cart2del_damp_3d.c \
	iem_roomsim.c

TARGET = iem_roomsim.pd_linux


OBJ = $(SRC:.c=.o) 

#
#  ------------------ targets ------------------------------------
#

clean:
	rm ..\$(TARGET)
	rm *.o

all: $(OBJ)
	@echo :: $(OBJ)
	$(LD) $(LDFLAGS) -o $(TARGET) *.o $(LIB)
	strip --strip-unneeded $(TARGET)
	mv $(TARGET) ..

$(OBJ) : %.o : %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c -o $*.o $*.c




