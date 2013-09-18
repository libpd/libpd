include Makefile.config

all: pdp_scaf.pd_linux

pdp_scaf_all:
	make -C include
	make -C compiler
	make -C rules
	make -C pdp

clean:
	rm -f *~
	rm -f pdp_scaf.pd_linux
	make -C include clean
	make -C compiler clean
	make -C rules clean
	make -C pdp clean

mrproper: clean
	rm -rf configure
	rm -rf config.status
	rm -rf config.log
	rm -rf autom4te.cache
	#this needs to stay in
	#rm -rf Makefile.config

pdp_scaf.pd_linux: pdp_scaf_all
	rm -f pdp_scaf.pd_linux
	gcc -rdynamic -shared -o pdp_scaf.pd_linux pdp/*.o $(PDP_CA_LIBS)

install: all
	install -d $(prefix)/lib/scaf
	install -m 755 compiler/scafc $(prefix)/bin || echo failed
	install -m 755 compiler/scafc.pl $(prefix)/lib/scaf || echo failed
	install -m 644 compiler/kernel.scaf $(prefix)/lib/scaf || echo failed
	install -m 644 compiler/scafmacro.s $(prefix)/lib/scaf || echo failed
	install -m 644 compiler/optim.rules $(prefix)/lib/scaf || echo failed
	install -m 755 rules/carules.scafo $(prefix)/lib/scaf/default.scafo || echo failed
	#Check if pd is installed in $(prefix)/lib/pd. 
	#If this fails the pdp_scaf lib and the docs won't be installed. 
	test -d $(prefix)/lib/pd
	install -m 755 pdp_scaf.pd_linux $(prefix)/lib/pd/extra
	install -m 644 doc/*.pd $(prefix)/lib/pd/doc/5.reference
