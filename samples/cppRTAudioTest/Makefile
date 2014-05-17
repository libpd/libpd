#
# Copyright (c) 2014 Rafael Vega <rvega@elsoftwarehamuerto.org>
# 
# BSD Simplified License.
# For information on usage and redistribution, and for a DISCLAIMER OF ALL
# WARRANTIES, see the file, "LICENSE.txt," in this distribution.
# 
# See https://github.com/libpd/libpd for documentation
#

SRC_FILES = src/PdObject.cpp src/main.cpp src/RtAudio.cpp
LIBPD = ../../libs/libpdcpp.so
TARGET = pdtest

CXXFLAGS = \
	-std=c++11 -O3 \
	-I../../pure-data/src \
	-I../../libpd_wrapper/util \
	-I../../libpd_wrapper \
	-I../../cpp \
	-D__UNIX_JACK__ \
	-D__LINUX_ALSA__ \
	-DLIBPD_USE_STD_MUTEX

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD)
	g++ $^ -o $(TARGET) -ljack -lasound -pthread $(LIBPD)

src/%.o: src/%.cpp 
	g++ $(CXXFLAGS) -c src/$*.cpp -o src/$*.o

$(LIBPD):
	cd ../.. && make cpplib

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)

