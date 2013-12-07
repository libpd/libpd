include ../../Makefile.config

OBJ = \
pixel_pack_s16u8.o \
pixel_unpack_u8s16.o \
pixel_add_s16.o \
pixel_mul_s16.o \
pixel_mix_s16.o \
pixel_randmix_s16.o \
pixel_conv_hor_s16.o \
pixel_conv_ver_s16.o \
pixel_biquad_s16.o \
pixel_ca_s1.o \
pixel_rand_s16.o \
pixel_crot_s16.o \
pixel_gain_s16.o \
pixel_resample_s16.o \
pixel_cheby_s16.o

all:	$(OBJ)

test:	pdp_mmx_test.o $(OBJ)
	gcc -o pdp_mmx_test pdp_mmx_test.o $(OBJ) -g

clean:
	rm -f *.o
	rm -f *~
	rm -f pdp_mmx.a
	rm -f pdp_mmx_test


