current: all_modules

include ../../Makefile.config

PDP_MOD = pdp_add.o pdp_conv.o \
	pdp_mix.o pdp_mul.o pdp_randmix.o \
	pdp_bq.o pdp_noise.o \
	pdp_gain.o pdp_zoom.o  \
	pdp_constant.o  \
	pdp_logic.o pdp_stateless.o pdp_plasma.o $(PDP_IMAGE_BASIC)


# build basic image processing modules (derived from base class)
all_modules: $(PDP_MOD)

clean:
	rm -f *~
	rm -f *.o

