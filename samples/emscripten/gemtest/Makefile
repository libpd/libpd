LIBPD_DIR = ../../../
GEM = Gem
SRC_FILES = gemtest.c
TARGET = gemtest.html

CFLAGS = -I$(LIBPD_DIR)/pure-data/src -I$(LIBPD_DIR)/libpd_wrapper -I$(GLU_DIR)/include -O3
LDFLAGS = -L$(LIBPD_DIR)/libs -lpd Gem/Gem.bc $(GLU_DIR)/lib/libGLU.a

$(TARGET): $(SRC_FILES) gem_init.c test.pd Makefile
	emcc $(CFLAGS) -o $(TARGET) $(SRC_FILES) \
	-DUSE_MGL_NAMESPACE=1 \
	-s USE_SDL=2 \
	-s USE_REGAL=1 \
	-s LEGACY_GL_EMULATION=0 \
	-s ERROR_ON_UNDEFINED_SYMBOLS=0 \
	--preload-file test.pd \
	--preload-file Gem/gemdefaultwindow.pd \
	--preload-file Gem/gemhead.pd \
	--preload-file Gem/gemkeyboard.pd \
	--preload-file Gem/gemkeyname.pd \
	--preload-file Gem/gemmouse.pd \
	--preload-file Gem/gemorb.pd \
	--preload-file Gem/gemtablet.pd \
	--preload-file Gem/gemwin.pd \
	--preload-file Gem/hsv2rgb.pd \
	--preload-file Gem/pix_blobtracker.pd \
	--preload-file Gem/pix_buffer_filmopen.pd \
	--preload-file Gem/pix_write.pd \
	--preload-file Gem/rgb2hsv.pd \
	--preload-file Gem/rgb2yuv.pd \
	--preload-file Gem/yuv2rgb.pd \
	$(LDFLAGS)
