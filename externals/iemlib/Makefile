# ###############################################
# LATER think about automatic adding of libraries
IEMLIBS = iemlib1 iemlib2 iem_t3_lib iem_mp3

# this is where we install to
PREFIX     =/usr/local/lib/pd

# this is where we install to (in detail)
INSTALL_BIN=$(PREFIX)/extra
INSTALL_DOC=$(PREFIX)/extra/iemhelp

# LATER think about separating iemabs and iemhelp

# ###############################################
.PHONY: iemlibs $(IEMLIBS)

iemlibs: $(IEMLIBS)

$(IEMLIBS):
	$(MAKE) -C $@/src

install: install-bin install-doc install-abs

install-bin:
	-install -d ${INSTALL_BIN}
	-install -m 644 lib/*.* $(INSTALL_BIN)

install-doc:
	-install -d ${INSTALL_DOC}
	-for d in ${IEMLIBS}; do \
	 for e in pd wav; do \
	  install -m644 $$d/*.$$e $(INSTALL_DOC); \
	 done; \
	done

install-abs:
	-install -d ${INSTALL_BIN}
	-install -m 644 iemabs/*.pd $(INSTALL_BIN)

clean:
	for d in ${IEMLIBS}; do \
	  ${MAKE} -C $$d/src clean; \
	done
