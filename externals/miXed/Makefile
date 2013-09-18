include Makefile.dirs

all:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) ; fi ) ; done
	@if [ -d doc/src ] ; then cd doc/src ; $(MAKE) ; fi

clean cleanall:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done
	rm -f *.gz

diff depend emptydeps:
	@for i in $(MIXED_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done

ALLSRC_TAR = release/miXed-`date +%F`-src.tar
ALLSRC_ROOTFILES = LICENSE.txt Makefile.common Makefile Makefile.dirs \
	dumpsetups

snap release:
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then cd $$i; $(MAKE) $@ ; fi ) ; done

rootsnap:
	tar -cf $(ALLSRC_TAR) $(ALLSRC_ROOTFILES)
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then tar -X $$i/$$i-all.exclude -rf \
			$(ALLSRC_TAR) $$i/* ; fi ) ; done
	tar -X shared/shared-all.exclude -rf $(ALLSRC_TAR) shared/*
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then tar -X $$i/$$i-test.exclude -rf \
			$(ALLSRC_TAR) test/$$i/* ; fi ) ; done
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then tar -T $$i/$$i-help.include -rf \
			$(ALLSRC_TAR) ; fi ) ; done
	@for i in $(RELEASE_DIRS) ; \
		do ( if [ -d $$i ] ; then tar -X $$i/$$i-vicious.exclude -rf \
			$(ALLSRC_TAR) ViCious/$$i/* ; fi ) ; done
	tar -rf $(ALLSRC_TAR) bin/notes.txt
	gzip -f $(ALLSRC_TAR)

fullsnap: snap rootsnap

backup:
	tar -X miXed-bak-exclude.files -zcf miXed-bak.tar.gz *
