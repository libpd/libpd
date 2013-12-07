current: all_modules

include ../../Makefile.config

PDP_MOD = pdp_reg.o pdp_del.o pdp_snap.o pdp_trigger.o \
	pdp_route.o pdp_inspect.o pdp_loop.o pdp_description.o pdp_convert.o \
	pdp_udp_send.o pdp_udp_receive.o pdp_rawin.o pdp_rawout.o pdp_metro.o

# build generic modules
all_modules: $(PDP_MOD)

clean:
	rm -f *~
	rm -f *.o

