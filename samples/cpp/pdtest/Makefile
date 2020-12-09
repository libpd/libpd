# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin) # Mac
  SOLIB_EXT = dylib
  PLATFORM = mac
  LDFLAGS = -stdlib=libc++
  CXXFLAGS = -stdlib=libc++
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    SOLIB_EXT = dll
    SOLIB_PREFIX =
    PLATFORM = windows
    LDLIBS = -Wl,--export-all-symbols -static-libgcc -lws2_32 -lkernel32
  else # assume Linux
    SOLIB_EXT = so
    PLATFORM = linux
    ifeq ($(UNAME), Linux)
      LDLIBS = -ldl
    endif
  endif
endif

LIBPD_DIR = ../../..
LIBPD = $(LIBPD_DIR)/libs/$(SOLIB_PREFIX)pd.$(SOLIB_EXT)

SRC_FILES = src/PdObject.cpp src/main.cpp
TARGET = pdtest

LDLIBS += -lm -lpthread
CXXFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper \
            -I$(LIBPD_DIR)/libpd_wrapper/util -I$(LIBPD_DIR)/cpp -I./src -O3

.PHONY: clean clobber static

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD)
	g++ -o $(TARGET) $^
ifeq ($(PLATFORM), mac)
	mkdir -p ./libs && cp $(LIBPD) ./libs
endif

static: CXXFLAGS += -DLIBPD_STATIC
static: ${SRC_FILES:.cpp=.o} $(LIBPD_DIR)/libs/libpd.a
	g++ -o $(TARGET) $^ $(LDLIBS)

$(LIBPD):
	cd $(LIBPD_DIR) && make UTIL=true EXTRA=true

$(LIBPD_DIR)/libs/libpd.a:
	cd $(LIBPD_DIR) && make UTIL=true EXTRA=true STATIC=true

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)
ifeq ($(PLATFORM), mac)
	rm -rf ./libs
endif
	cd $(LIBPD_DIR) && make clobber
