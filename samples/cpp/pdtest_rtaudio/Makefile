#
# Copyright (c) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
# 
# BSD Simplified License.
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "LICENSE.txt," in this distribution.
# 
# See https://github.com/libpd/libpd for documentation
#

# detect platform, move libpd dylib to local folder on mac
UNAME = $(shell uname)
SOLIB_PREFIX = lib

ifeq ($(UNAME), Darwin)  # Mac
  SOLIB_EXT = dylib
  PDNATIVE_PLATFORM = mac
  CXXFLAGS = -D__MACOSX_CORE__
  AUDIO_API = -framework Foundation -framework CoreAudio
else
  ifeq ($(OS), Windows_NT)  # Windows, use Mingw
    SOLIB_EXT = dll
    SOLIB_PREFIX = 
    PDNATIVE_PLATFORM = windows
    CXXFLAGS = -D__WINDOWS_DS__
    AUDIO_API = -lole32 -loleaut32 -ldsound -lwinmm
  else  # Assume Linux
    SOLIB_EXT = so
    PDNATIVE_PLATFORM = linux
    CXXFLAGS = -D__UNIX_JACK__ -D__LINUX_ALSA__
    AUDIO_API = -ljack -lasound -pthread
  endif
endif

LIBPD_DIR = ../../..
LIBPD_CPP = $(LIBPD_DIR)/libs/libpdcpp.$(SOLIB_EXT)

SRC_FILES = src/PdObject.cpp src/main.cpp src/RtAudio.cpp
TARGET = pdtest_rtaudio

CXXFLAGS += -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -I$(LIBPD_DIR)/cpp -I./src -std=c++11 -DLIBPD_USE_STD_MUTEX -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD_CPP)
	g++ -o $(TARGET) $^ $(LIBPD_CPP) $(AUDIO_API)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then mkdir -p ./libs && cp $(LIBPD_CPP) ./libs; fi

$(LIBPD_CPP):
	cd $(LIBPD_DIR) && make cpplib UTIL=true EXTRA=true

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)
	if [ $(PDNATIVE_PLATFORM) == "mac" ]; then rm -rf ./libs; fi
