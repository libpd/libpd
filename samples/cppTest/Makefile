SRC_FILES = src/PdObject.cpp src/main.cpp ../../cpp/PdBase.cpp ../../cpp/PdTypes.cpp
LIBPD = ../../libs/libpd.so
TARGET = pdtest

CXXFLAGS = -std=c++11 -DUSE_STD_MUTEX -I../../pure-data/src -I../../libpd_wrapper/util -I../../libpd_wrapper -I../../cpp -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD)
	g++ $^ $(LIBPD) -o $(TARGET)

src/%.o: src/%.cpp 
	g++ $(CXXFLAGS) -c src/$*.cpp -o src/$*.o

$(LIBPD):
	cd ../.. && make

clean:
	rm -f src/*.o
	rm -f ../../cpp/*.o

clobber: clean
	rm -f $(TARGET)

