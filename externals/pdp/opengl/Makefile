include Makefile.config

all:	$(TARGET)

linux:	pdp_opengl.pd_linux

darwin:	pdp_opengl.pd_darwin

subdirs:
	make -C system
	make -C modules
	make -C include

clean:
	make -C system clean
	make -C modules clean
	make -C include clean
	rm -f pdp_opengl.pd_linux
	rm -f *~

pdp_opengl.pd_linux: subdirs
	rm -f pdp_opengl.pd_linux
	$(CC) -rdynamic -shared -o pdp_opengl.pd_linux modules/*.o system/*.o $(LDFLAGS) -g

pdp_opengl.pd_darwin: subdirs
	rm -f pdp_opengl.pd_linux
	$(CC) $(LDFLAGS) -o pdp_opengl.pd_darwin ../modules/*/*.o ../system/pdp.o ../system/*/*.o ../puredata/*.o modules/*.o system/*.o $(PDP_LIBS) -bundle -undefined dynamic_lookup -bundle_loader $(PD_EXECUTABLE)


# $(CC) -o pdp_opengl.pd_pd_darwin modules/*.o system/*.o $(LDFLAGS) -g -dynamic -bundle -flat_namespace -undefined suppress
# $(CC) -o pdp_opengl.pd_pd_darwin modules/*.o system/*.o $(LDFLAGS) -g -bundle -bundle_loader $(PD_EXECUTABLE)

install: all
	test -d $(prefix)/lib/pd 	#check if pd is installed. if this fails make install will stop here.
	install -d $(prefix)/lib/pd/extra
	install -m 755 $(PDP_OPENGL_LIBRARY_NAME) $(prefix)/lib/pd/extra
	install -m 644 abstractions/*.pd $(prefix)/lib/pd/extra
	install -m 644 doc/objects/*.pd $(prefix)/lib/pd/doc/5.reference
	install -m 755 -d $(prefix)/lib/pd/doc/pdp
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/introduction
	install -m 755 -d $(prefix)/lib/pd/doc/pdp/examples_opengl
	install -m 644 doc/reference_opengl.txt $(prefix)/lib/pd/doc/pdp
	install -m 644 doc/examples/*.pd $(prefix)/lib/pd/doc/pdp/examples_opengl

