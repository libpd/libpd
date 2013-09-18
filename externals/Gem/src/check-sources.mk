# check for forgotten sources
# include this makefile snippet in your Makefile.am and it will print-out a list
# of all .cpp and .h files that have been forgotten to be added to DIST_SOURCES

.PHONY: check-sources

all-local:: check-sources

_check_sources_all=$(wildcard *.cpp *.h)
_check_sources_unused=$(filter-out $(DIST_SOURCES) $(CHECKUNUSED_IGNORE), $(_check_sources_all))

check-sources: 
	@echo "UNUSED SOURCES: $(_check_sources_unused)"


