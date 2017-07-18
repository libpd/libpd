# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin) # Mac
  SOLIB_EXT = dylib
  PLATFORM = mac
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    SOLIB_EXT = dll
    PLATFORM = windows
  else # assume Linux
    SOLIB_EXT = so
    PLATFORM = linux
  endif
endif

LIBPD_DIR = ../../..
LIBPD = $(LIBPD_DIR)/libs/libpd.$(SOLIB_EXT)

SRC_FILES = pdtest.c
TARGET = pdtest

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)
	if [ $(PLATFORM) == "mac" ]; then mkdir -p ./libs && cp $(LIBPD) ./libs; fi

$(LIBPD):
	cd $(LIBPD_DIR) && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PLATFORM) == "mac" ]; then rm -rf ./libs; fi
	cd $(LIBPD_DIR) && make clobber
