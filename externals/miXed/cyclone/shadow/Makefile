ROOT_DIR = ../..
redefault: default $(ROOT_DIR)/bin/cyclist
include $(ROOT_DIR)/Makefile.common

$(ROOT_DIR)/bin/cyclist: $(SHARED_DIR)/common/binport.c \
	$(SHARED_DIR)/common/lex.c $(SHARED_DIR)/unstable/standalone.c 
	$(CC) $(CFLAGS) -DMIXED_STANDALONE -o $@ $^
