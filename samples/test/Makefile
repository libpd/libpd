SRC_FILES = pdtest.c
TARGET = pdtest

CFLAGS = -I../../pure-data/src

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o}
	gcc -o $(TARGET) $^ ../../pure-data/src/libpd.so

clean:
	-rm *.o

clobber: clean
	-rm $(TARGET)

