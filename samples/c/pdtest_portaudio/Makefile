# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin) # Mac
  SOLIB_EXT = dylib
  PLATFORM = mac
  # homebrew paths
  PA_CFLAGS = -I/opt/homebrew/include
  PA_LDFLAGS = -L/opt/homebrew/lib -lportaudio
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    SOLIB_EXT = dll
    PLATFORM = windows
  else # assume Linux
    SOLIB_EXT = so
    PLATFORM = linux
  endif
  PA_LDFLAGS = -lportaudio
endif

LIBPD_DIR = ../../..
LIBPD_CFLAGS = -I$(LIBPD_DIR)/libpd_wrapper -I$(LIBPD_DIR)/pure-data/src
LIBPD = $(LIBPD_DIR)/libs/libpd.$(SOLIB_EXT)

SRC_FILES = pdtest_portaudio.c
TARGET = pdtest_portaudio

CFLAGS = $(LIBPD_CFLAGS) $(PA_CFLAGS) -O3
LDFLAGS = $(PA_LDFLAGS) $(LIBPD)

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.c=.o} $(LIBPD)
	gcc -o $(TARGET) $^ $(LDFLAGS)
ifeq ($(PLATFORM), mac)
	mkdir -p ./libs && cp $(LIBPD) ./libs
endif

$(LIBPD):
	cd $(LIBPD_DIR) && make

clean:
	rm -f *.o

clobber: clean
	rm -f $(TARGET)
ifeq ($(PLATFORM), mac)
	rm -rf ./libs
endif
	cd $(LIBPD_DIR) && make clobber
