# convenience symlinks for extra_LTLIBRARIES

## uha, this is ugly
%.@GEM_RTE_EXTENSION@: %.la
	rm -f $@
	test -f .libs/$@ && $(LN_S) .libs/$@ $@ || true

.PHONY: clean-conviencesymlink

clean-conviencesymlink:
	rm -f *.@GEM_RTE_EXTENSION@


all-local:: $(extra_LTLIBRARIES:.la=.@GEM_RTE_EXTENSION@)

clean-local:: clean-conviencesymlink
