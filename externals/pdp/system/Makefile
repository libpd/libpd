
include ../Makefile.config

all: $(PDP_TARGET) pdp.o

common:
	make -C net
	make -C X11
	make -C png
	make -C type
	make -C kernel
	make -C image $(PDP_TARGET)

linux_mmx: common
	make -C mmx

linux: common

darwin: common

clean:
	rm -f *~
	rm -f *.o
	make -C mmx clean
	make -C net clean
	make -C X11 clean
	make -C png clean
	make -C image clean
	make -C type clean
	make -C kernel clean
