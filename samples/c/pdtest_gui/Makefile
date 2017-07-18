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

SRC_FILES = pdtest_gui.c
TARGET = pdtest_gui

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
         -Wall -Werror -O3 -DPDINSTANCE

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD) $(LDFLAGS)
	if [ $(PLATFORM) == "mac" ]; then mkdir -p ./libs && cp $(LIBPD) ./libs; fi

$(LIBPD):
	cd $(LIBPD_DIR) && make ADDITIONAL_CFLAGS=-DPDINSTANCE

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PLATFORM) == "mac" ]; then rm -rf ./libs; fi
	cd $(LIBPD_DIR) && make clobber
