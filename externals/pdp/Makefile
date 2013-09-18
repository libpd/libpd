include Makefile.config

# hc ?
#ifeq ($(OS_NAME),darwin)
#all: darwin
#else
#all: linux
#endif


all: $(PDP_TARGET)

everything: all
	make -C opengl
	make -C scaf

pdp_all:
	make -C system
	make -C puredata
	make -C modules

darwin:	pdp_all
	rm -f pdp.pd_darwin
	$(CC)  -o pdp.pd_darwin modules/*/*.o system/pdp.o system/*/*.o puredata/*.o $(PDP_LIBS) -bundle -undefined  dynamic_lookup  -twolevel_namespace -bundle_loader $(PD_EXECUTABLE) 


linux: pdp_all
	rm -f pdp.pd_linux
	$(CC) -rdynamic -shared -o pdp.pd_linux modules/*/*.o system/pdp.o system/*/*.o puredata/*.o $(PDP_LIBS)

linux_mmx: pdp_all
	rm -f pdp.pd_linux
	$(CC) -rdynamic -shared -o pdp.pd_linux modules/*/*.o system/pdp.o system/*/*.o puredata/*.o $(PDP_LIBS)

buildclean:
	make -C include clean
	make -C system clean
	make -C puredata clean
	make -C modules clean

backupclean:
	rm -f *~ */*~ */*/*~

clean: buildclean backupclean
	rm -f pdp.pd_linux

distroclean: buildclean
	make -C scaf clean
	make -C opengl clean

mrproper: clean distroclean
	make -C scaf mrproper
	rm -rf configure
	rm -rf config.log
	rm -rf config.status
	rm -rf autom4te.cache
	#this needs to stay in to keep the makefiles working
	#rm -rf Makefile.config

tags:
	etags --language=auto include/*.h system/*.c system/mmx/*.s system/*/*.c puredata/*.c \
	modules/*/*.c scaf/*/*.c scaf/*/*.s opengl/*/*.c

tagsclean:
	rm -f TAGS


install: all
	#check if pd is installed. if this fails make install will stop here.
	test -d $(prefix)/lib/pd
	install -d $(prefix)/lib/pd/extra
	install -m 755 $(PDP_LIBRARY_NAME) $(prefix)/lib/pd/extra
	install -m 755 -d $(prefix)/include/pdp
	install -m 644 include/*.h $(prefix)/include/pdp
	install -m 644 abstractions/*.pd $(prefix)/lib/pd/extra
	install -m 644 doc/objects/*.pd $(prefix)/lib/pd/doc/5.reference
	install -m 755 -d $(prefix)/lib/pd/doc/pdp
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/introduction
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/examples
	install -m 644 doc/reference.txt $(prefix)/lib/pd/doc/pdp
	install -m 644 doc/introduction/*.pd $(prefix)/lib/pd/doc/pdp/introduction
	install -m 644 doc/examples/*.pd $(prefix)/lib/pd/doc/pdp/examples
	install -m 755 bin/pdp-config $(prefix)/bin

release:
	bin/snapshot `bin/release-version`
	echo bump PDP_VERSION in configure.ac!
