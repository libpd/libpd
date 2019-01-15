LIBPD_DIR = ../../../
SRC_FILES = gemtest.c glu.c
TARGET = gemtest.html

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -O3
LDFLAGS = -L$(LIBPD_DIR)/libs -lpd Gem.bc

$(TARGET): $(SRC_FILES) test.pd Makefile
	emcc $(CFLAGS) -o $(TARGET) $(SRC_FILES) \
	-DUSE_MGL_NAMESPACE=1 \
	-s USE_SDL=2 \
	-s USE_REGAL=1 \
	-s LEGACY_GL_EMULATION=0 \
	-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
	--preload-file test.pd \
	--preload-file Gem \
	$(LDFLAGS)
