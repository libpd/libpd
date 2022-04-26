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

SRC_FILES = pdtest_double.c
TARGET = pdtest_double

CFLAGS = -I$(LIBPD_DIR)/libpd_wrapper -I$(LIBPD_DIR)/pure-data/src \
         -DPD_FLOATSIZE=64 -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LIBPD)
ifeq ($(PLATFORM), mac)
	mkdir -p ./libs && cp $(LIBPD) ./libs
endif

$(LIBPD):
	cd $(LIBPD_DIR) && make DOUBLE=true

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
ifeq ($(PLATFORM), mac)
	rm -rf ./libs
endif
	cd $(LIBPD_DIR) && make clobber
