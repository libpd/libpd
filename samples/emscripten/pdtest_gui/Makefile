LIBPD_DIR = ../../../

SRC_FILES = pdtest_gui.c
TARGET = pdtest_gui.html

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -O3
LDFLAGS = -L$(LIBPD_DIR)/build/libs -lpd

.PHONY: clean clobber

$(TARGET): $(SRC_FILES) test.pd
	emcc $(CFLAGS) -o $(TARGET) $(SRC_FILES) --closure 1 -s USE_SDL=2 -s ERROR_ON_UNDEFINED_SYMBOLS=0 --preload-file test.pd $(LDFLAGS)
