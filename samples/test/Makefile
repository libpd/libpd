SRC_FILES = pdtest.c
TARGET = pdtest

CFLAGS = -I../../pure-data/src -I../../libpd_wrapper

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o}
	gcc -o $(TARGET) $^ ../../libpd.so

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)

