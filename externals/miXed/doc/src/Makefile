include Makefile.dirs

default: all

.DEFAULT:
	@for SUB_DIR in $(SUB_DIRS) ; \
		do ( if [ -d $$SUB_DIR ] && [ -f $$SUB_DIR/Makefile ] ; then \
			cd $$SUB_DIR; $(MAKE) $@ ; fi ) ; done
