SRC_FILES = libpdstest.c
LIBPD = ../../libs/libpds.so
TARGET = libpdstest

CFLAGS = -I../../pure-data/src -I../../libpd_wrapper -O3 -g -Wall

.PHONY: clean clobber

all:
	cd ../.. && make
	make $(TARGET)

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -Wall -o $(TARGET) $^

$(LIBPD):
	cd ../.. && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)

