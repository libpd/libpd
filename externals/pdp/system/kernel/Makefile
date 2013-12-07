
OBJECTS = pdp_packet.o pdp_type.o pdp_dpd_command.o \
	pdp_list.o pdp_debug.o pdp_symbol.o \
	pdp_mem.o pdp_post.o


include ../../Makefile.config

all: pdp_main_clean $(OBJECTS)

pdp_main_clean:
	rm -f pdp.o

clean:
	rm -f *~
	rm -f *.o
