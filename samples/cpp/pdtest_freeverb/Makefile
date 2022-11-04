# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin) # Mac
  SOLIB_EXT = dylib
  PLATFORM = mac
  CXXFLAGS = -D__MACOSX_CORE__ -DHAVE_UNISTD_H
  AUDIO_API = -framework Foundation -framework CoreAudio
else
  ifeq ($(OS), Windows_NT) # Windows, use Mingw
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PLATFORM = windows
    CXXFLAGS = -D__WINDOWS_DS__
    AUDIO_API = -lole32 -loleaut32 -ldsound -lwinmm
  else # assume Linux
    SOLIB_EXT = so
    PLATFORM = linux
    CXXFLAGS = -D__UNIX_JACK__ -D__LINUX_ALSA__
    AUDIO_API = -ljack -lasound -pthread
  endif
endif

LIBPD_DIR = ../../..
LIBPD = $(LIBPD_DIR)/libs/libpd.$(SOLIB_EXT)

SRC_FILES = src/PdObject.cpp src/RtAudio.cpp src/main.cpp
EXT_FILES = src/externals/freeverb~.c
TARGET = pdtest_freeverb

# make sure to define PD for freeverb~
CFLAGS = -I$(LIBPD_DIR)/libpd_wrapper -I$(LIBPD_DIR)/libpd_wrapper/util \
         -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/cpp -I./src -DPD -O3
CXXFLAGS += $(CFLAGS) -std=c++11 -DLIBPD_USE_STD_MUTEX

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} ${EXT_FILES:.c=.o} $(LIBPD)
	g++ -o $(TARGET) $^ $(CXXFLAGS) $(LIBPD) $(AUDIO_API)
ifeq ($(PLATFORM), mac)
	mkdir -p ./libs && cp $(LIBPD) ./libs
endif

$(LIBPD):
	cd $(LIBPD_DIR) && make

clean:
	rm -f src/*.o
	rm -f src/externals/*.o

clobber: clean
	rm -f $(TARGET)
ifeq ($(PLATFORM), mac)
	rm -rf ./libs
endif
	cd $(LIBPD_DIR) && make clobber
