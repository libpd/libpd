include ../../Makefile.config

all: $(PDP_TARGET)

OBJECTS = pdp_llconv.o pdp_resample.o pdp_imageproc_common.o

OBJECTS_MMX = pdp_imageproc_mmx.o pdp_llconv_mmx.o
OBJECTS_PORTABLE =  pdp_imageproc_portable.o pdp_llconv_portable.o




linux_mmx: $(OBJECTS_MMX) $(OBJECTS)

linux: $(OBJECTS_PORTABLE) $(OBJECTS)

darwin: $(OBJECTS_PORTABLE) $(OBJECTS)

clean:
	rm -f *~
	rm -f *.o
