LIBPD_DIR = ../../../
GLU_DIR = ../../../../glu/.libs/
GEM_DIR = ../../../../Gem/
SRC_FILES = gemtest.c glx.c
TARGET = gemtest.html

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -O3
LDFLAGS = -L$(LIBPD_DIR)/build/libs -lpd -lGL -lregal -lGLEW -Wl,--whole-archive $(GEM_DIR)/src/Gem.a -Wl,--no-whole-archive $(GLU_DIR)/libGLU.a -lGLEW

$(TARGET): $(SRC_FILES) test.pd Makefile Gem/Gem-meta.pd
	emcc $(CFLAGS) -o $(TARGET) $(SRC_FILES) \
	-s USE_SDL=2 \
	-s USE_REGAL=1 \
	-s LEGACY_GL_EMULATION=0 \
	-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
	--preload-file test.pd \
	--preload-file Gem \
	$(LDFLAGS)

Gem/Gem-meta.pd:
	mkdir -p Gem/
	cp -t Gem/ $(GEM_DIR)/Gem-meta.pd $(GEM_DIR)/abstractions/*.pd
