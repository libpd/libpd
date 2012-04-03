SRC_FILES = pdtest.c
LIBPD = ../../../libs/libpd.so
TARGET = pdtest

CFLAGS = -I../../../pure-data/src -I../../../libpd_wrapper -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)

$(LIBPD):
	cd ../../.. && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)

