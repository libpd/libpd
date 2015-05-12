# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_PLATFORM = mac
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    SOLIB_EXT = dll
    PDNATIVE_PLATFORM = windows
  else  # Assume Linux
    SOLIB_EXT = so
    PDNATIVE_PLATFORM = linux
  endif
endif

LIBPD_DIR = ../../..
LIBPD = $(LIBPD_DIR)/libs/libpd.$(SOLIB_EXT)

SRC_FILES = pdtest.c
TARGET = pdtest

CFLAGS = -I../../../pure-data/src -I../../../libpd_wrapper -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then mkdir -p ./libs && cp $(LIBPD) ./libs; fi

$(LIBPD):
	cd ../../.. && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then rm -rf ./libs; fi
