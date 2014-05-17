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

