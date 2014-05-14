SRC_FILES = src/PdObject.cpp src/main.cpp
LIBPD = ../../libs/libpdcpp.so
TARGET = pdtest

CXXFLAGS = -I../../pure-data/src -I../../libpd_wrapper -I../../cpp -O3

.PHONY: clean clobber

$(TARGET): ${SRC_FILES:.cpp=.o} $(LIBPD)
	g++ $^ $(LIBPD) -o $(TARGET)

src/%.o: src/%.cpp 
	g++ $(CXXFLAGS) -c src/$*.cpp -o src/$*.o

$(LIBPD):
	cd ../.. && make cpplib

clean:
	rm -f src/*.o

clobber: clean
	rm -f $(TARGET)

