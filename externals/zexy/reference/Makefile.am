AUTOMAKE_OPTIONS = foreign

SUFFIXES = .pd

zexydir=$(pkglibdir)

dist_zexy_DATA= \
	a2l-help.pd \
	atoi-help.pd \
	avg~-help.pd \
	blockmirror~-help.pd \
	blockswap~-help.pd \
	coordinates-help.pd \
	date-help.pd \
	demultiplex~-help.pd \
	demultiplex-help.pd \
	dfreq~-help.pd \
	digidistort-help.pd \
	dirac~-help.pd \
	drip-help.pd \
	envrms~-help.pd \
	fifop-help.pd \
	freadln-help.pd \
	fwriteln-help.pd \
	glue-help.pd \
	index-help.pd \
	length-help.pd \
	lifop-help.pd \
	limiter~-help.pd \
	list2int-help.pd \
	list2lists-help.pd \
	list2symbol-help.pd \
	lister-help.pd \
	listfind-help.pd \
	lpt-help.pd \
	makesymbol-help.pd \
	matchbox-help.pd \
	mavg-help.pd \
	minmax-help.pd \
	msgfile-help.pd \
	multiline~-help.pd \
	multiplex~-help.pd \
	multiplex-help.pd \
	niagara-help.pd \
	noish~-help.pd \
	noisi~-help.pd \
	operating_system-help.pd \
	packel-help.pd \
	pack~-help.pd \
	pack-help.pd \
	pdf~-help.pd \
	prime-help.pd \
	quantize~-help.pd \
	rawprint-help.pd \
	regex-help-help.pd \
	regex-help.pd \
	relay-help.pd \
	repack-help.pd \
	repeat-help.pd \
	route~-help.pd \
	scalarmult-help.pd \
	sf-play_record-help.pd \
	sigzero~-help.pd \
	sort-help.pd \
	step~-help.pd \
	strcmp-help.pd \
	sum-help.pd \
	swap~-help.pd \
	symbol2list-help.pd \
	tabdump-help.pd \
	tabminmax-help.pd \
	tabset-help.pd \
	tavg~-help.pd \
	time-help.pd \
	unpack~-help.pd \
	unpack-help.pd \
	urn-help.pd \
	wrap-help.pd \
	z~-help.pd \
	zigbinops-help.pd

#############
# symlinks for aliases
zexyaliasdir=$(pkglibdir)

clean-local: clean_zexyalias
.PHONY: clean_zexyalias

dist_zexyalias_DATA= \
	unfold-help.pd \
	mux-help.pd \
	demux~-help.pd \
	l2i-help.pd \
	l2s-help.pd \
	any2list-help.pd \
	lp-help.pd \
	demux-help.pd \
	l-help.pd \
	mux~-help.pd \
	s2l-help.pd

# create aliases
$(dist_zexyalias_DATA):
	test -e $< && $(LN_S) $< $@

# delete aliases
clean_zexyalias:
	-rm -f $(dist_zexyalias_DATA)

# define aliases
any2list-help.pd: a2l-help.pd
demux~-help.pd: demultiplex~-help.pd
demux-help.pd: demultiplex-help.pd
l2i-help.pd: list2int-help.pd
l2s-help.pd: list2symbol-help.pd
l-help.pd: lister-help.pd
lp-help.pd: lpt-help.pd
mux~-help.pd: multiplex~-help.pd
mux-help.pd: multiplex-help.pd
s2l-help.pd: symbol2list-help.pd
unfold-help.pd: drip-help.pd
