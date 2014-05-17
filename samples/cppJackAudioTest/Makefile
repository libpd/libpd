# 
# copyright (c) 2014 rafael vega <rvega@elsoftwarehamuerto.org>
# 
# bsd simplified license.
# for information on usage and redistribution, and for a disclaimer of all
# warranties, see the file, "license.txt," in this distribution.
# 
# see https://github.com/libpd/libpd for documentation
#

SRC_FILES = src/main.cpp src/PdObject.cpp 
LIBPD = ../../libs/libpdcpp.so
TARGET = pd-jack-test

CXXFLAGS = -I../../pure-data/src -I../../libpd_wrapper/util -I../../libpd_wrapper -I../../cpp -std=c++11 -DLIBPD_USE_STD_MUTEX -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD)
	g++ $^ -o $(TARGET) -ljack $(LIBPD)

src/%.o: src/%.cpp 
	g++ $(CXXFLAGS) -c src/$*.cpp -o src/$*.o

$(LIBPD):
	cd ../.. && make cpplib

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)

