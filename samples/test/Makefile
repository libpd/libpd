SRC_FILES = pdtest.c
LIBPD = ../../libpd.so
TARGET = pdtest

CFLAGS = -I../../pure-data/src -I../../libpd_wrapper

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)

