SRC_FILES = src/main.cpp src/PdObject.cpp ../../cpp/PdBase.cpp ../../cpp/PdTypes.cpp
LIBPD = ../../libs/libpd.so
TARGET = pd-test

CXXFLAGS = -I../../pure-data/src -I../../libpd_wrapper/util -I../../libpd_wrapper -I../../cpp -std=c++11 -DLIBPD_USE_STD_MUTEX -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o}
	g++ $^ -o $(TARGET) -ljack $(LIBPD)

src/%.o: src/%.cpp 
	g++ $(CXXFLAGS) -c src/$*.cpp -o src/$*.o

$(LIBPD):
	cd ../.. && make

clean:
	rm -f src/*.o
	rm -f ../../cpp/*.o

clobber: clean
	rm -f $(TARGET)

