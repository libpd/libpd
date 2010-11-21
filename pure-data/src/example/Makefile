SRC_FILES = pdtest.c
TARGET = pdtest

CFLAGS = -I..

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o}
	gcc -o $(TARGET) $^ ../libpd.so

clean:
	-rm *.o

clobber: clean
	-rm $(TARGET)

