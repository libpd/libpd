SRC_FILES = guitest.c
LIBPD = ../../libs/libpd.so
TARGET = guitest

CFLAGS = -I../../pure-data/src -I../../libpd_wrapper -O3

.PHONY: clean clobber

all:
	cd ../.. && make libs/libpd.so
	make $(TARGET)

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)

$(LIBPD):
	cd ../.. && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)

