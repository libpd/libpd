# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_PLATFORM = mac
  LDFLAGS = -stdlib=libc++
  CXXFLAGS = -stdlib=libc++
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PDNATIVE_PLATFORM = windows
  else  # Assume Linux
    SOLIB_EXT = so
    PDNATIVE_PLATFORM = linux
  endif
endif

LIBPD_DIR = ../../..
LIBPD_CPP = $(LIBPD_DIR)/libs/libpdcpp.$(SOLIB_EXT)

SRC_FILES = src/PdObject.cpp src/main.cpp
TARGET = pdtest

CXXFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -I$(LIBPD_DIR)/cpp -I./src -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD_CPP)
	g++ -o $(TARGET) $^ $(LIBPD_CPP)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then mkdir -p ./libs && cp $(LIBPD_CPP) ./libs; fi

$(LIBPD_CPP):
	cd $(LIBPD_DIR) && make cpplib UTIL=true EXTRA=true

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then rm -rf ./libs; fi
