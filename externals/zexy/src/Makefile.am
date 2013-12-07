AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

pkglib_LTLIBRARIES=

AM_CFLAGS =
AM_LDFLAGS= -module -avoid-version -shared -shrext .@EXTENSION@

if WINDOWS
AM_LDFLAGS+= -no-undefined
endif

AM_LDFLAGS+= @EXTRA_LTFLAGS@

# RTE
AM_CFLAGS += 
AM_LDFLAGS+= 


if LIBRARY
pkglib_LTLIBRARIES+=zexy.la
endif LIBRARY
if SINGLEOBJECTS
pkglib_LTLIBRARIES+= \
	0x260x260x7e.la \
	0x2e.la \
	0x3c0x7e.la \
	0x3d0x3d0x7e.la \
	0x3e0x7e.la \
	0x7c0x7c0x7e.la \
	any2list.la \
	a2l.la \
	abs~.la \
	absgn~.la \
	atoi.la \
	avg~.la \
	blockmirror~.la \
	blockshuffle~.la \
	blockswap~.la \
	date.la \
	demultiplex~.la \
	demux~.la \
	demultiplex.la \
	demux.la \
	dfreq~.la \
	dirac~.la \
	drip.la \
	unfold.la \
	envrms~.la \
	fifop.la \
	freadln.la \
	fwriteln.la \
	glue.la \
	index.la \
	length.la \
	lifop.la \
	limiter~.la \
	l2i.la \
	list2int.la \
	list2lists.la \
	l2s.la \
	list2symbol.la \
	l.la \
	lister.la \
	listfind.la \
	liststorage.la \
	lpt.la \
	lp.la \
	makesymbol.la \
	matchbox.la \
	mavg.la \
	minmax.la \
	msgfile.la \
	multiline~.la \
	mux~.la \
	mux.la \
	multiplex~.la \
	multiplex.la \
	multireceive.la \
	niagara.la \
	noish~.la \
	noisi~.la \
	operating_system.la \
	pack~.la \
	pack.la \
	packel.la \
	pdf~.la \
	prime.la \
	quantize~.la \
	rawprint.la \
	regex.la \
	relay.la \
	repack.la \
	repeat.la \
	route~.la \
	sfplay.la \
	sfrecord.la \
	sgn~.la \
	sigzero~.la \
	sleepgrain.la \
	sort.la \
	step~.la \
	strcmp.la \
	sum.la \
	swap~.la \
	symbol2list.la \
	s2l.la \
	tabdump.la \
	tabminmax.la \
	tabread4~~.la \
	tabset.la \
	tavg~.la \
	time.la \
	unpack~.la \
	unpack.la \
	urn.la \
	wrap.la \
	z~.la
endif SINGLEOBJECTS

zexy_la_CPPFLAGS=-DZEXY_LIBRARY


0x260x260x7e_la_SOURCES	= 0x260x260x7e.c
0x2e_la_SOURCES		= 0x2e.c
0x3c0x7e_la_SOURCES	= 0x3c0x7e.c
0x3d0x3d0x7e_la_SOURCES	= 0x3d0x3d0x7e.c
0x3e0x7e_la_SOURCES	= 0x3e0x7e.c
0x7c0x7c0x7e_la_SOURCES	= 0x7c0x7c0x7e.c
a2l_la_SOURCES		= a2l.c
any2list_la_SOURCES	= a2l.c
abs__la_SOURCES		= abs~.c
absgn__la_SOURCES	= absgn~.c
atoi_la_SOURCES		= atoi.c
avg__la_SOURCES		= avg~.c
blockmirror__la_SOURCES	= blockmirror~.c
blockshuffle__la_SOURCES= blockshuffle~.c
blockswap__la_SOURCES	= blockswap~.c
date_la_SOURCES		= date.c
demultiplex__la_SOURCES	= demultiplex~.c
demux__la_SOURCES	= demultiplex~.c
demultiplex_la_SOURCES	= demultiplex.c
demux_la_SOURCES	= demultiplex.c
dfreq__la_SOURCES	= dfreq~.c
dirac__la_SOURCES	= dirac~.c
drip_la_SOURCES		= drip.c
unfold_la_SOURCES	= drip.c
envrms__la_SOURCES	= envrms~.c
fifop_la_SOURCES	= fifop.c
freadln_la_SOURCES	= freadln.c
fwriteln_la_SOURCES	= fwriteln.c
glue_la_SOURCES		= glue.c
index_la_SOURCES	= index.c
length_la_SOURCES	= length.c
lifop_la_SOURCES	= lifop.c
limiter__la_SOURCES	= limiter~.c
list2int_la_SOURCES	= list2int.c
l2i_la_SOURCES		= list2int.c
list2lists_la_SOURCES	= list2lists.c
list2symbol_la_SOURCES	= list2symbol.c
l2s_la_SOURCES		= list2symbol.c
lister_la_SOURCES	= lister.c
l_la_SOURCES		= lister.c
listfind_la_SOURCES	= listfind.c
liststorage_la_SOURCES	= liststorage.c
if LPT
lpt_la_SOURCES		= lpt.c
lp_la_SOURCES		= lpt.c
if WINDOWS
lpt_la_SOURCES		+= winNT_portio.c
lp_la_SOURCES		+= winNT_portio.c
endif
endif
makesymbol_la_SOURCES	= makesymbol.c
matchbox_la_SOURCES	= matchbox.c
mavg_la_SOURCES		= mavg.c
minmax_la_SOURCES	= minmax.c
msgfile_la_SOURCES	= msgfile.c
multiline__la_SOURCES	= multiline~.c
mux__la_SOURCES		= multiplex~.c
multiplex__la_SOURCES	= multiplex~.c
mux_la_SOURCES		= multiplex.c
multiplex_la_SOURCES	= multiplex.c
multireceive_la_SOURCES	= multireceive.c
niagara_la_SOURCES	= niagara.c
noish__la_SOURCES	= noish~.c
noisi__la_SOURCES	= noisi~.c
operating_system_la_SOURCES	= operating_system.c
pack__la_SOURCES	= pack~.c
pack_la_SOURCES		= pack.c
packel_la_SOURCES	= packel.c
pdf__la_SOURCES		= pdf~.c
prime_la_SOURCES	= prime.c
quantize__la_SOURCES	= quantize~.c
rawprint_la_SOURCES	= rawprint.c
regex_la_SOURCES	= regex.c
relay_la_SOURCES	= relay.c
repack_la_SOURCES	= repack.c
repeat_la_SOURCES	= repeat.c
route__la_SOURCES	= route~.c
sfplay_la_SOURCES	= sfplay.c
sfrecord_la_SOURCES	= sfrecord.c
sgn__la_SOURCES		= sgn~.c
sigzero__la_SOURCES	= sigzero~.c
sleepgrain_la_SOURCES	= sleepgrain.c
sort_la_SOURCES		= sort.c
step__la_SOURCES	= step~.c
strcmp_la_SOURCES	= strcmp.c
sum_la_SOURCES		= sum.c
swap__la_SOURCES	= swap~.c
symbol2list_la_SOURCES	= symbol2list.c
s2l_la_SOURCES		= symbol2list.c
tabdump_la_SOURCES	= tabdump.c
tabminmax_la_SOURCES	= tabminmax.c
tabread4___la_SOURCES	= tabread4~~.c
tabset_la_SOURCES	= tabset.c
tavg__la_SOURCES	= tavg~.c
time_la_SOURCES		= time.c
unpack__la_SOURCES	= unpack~.c
unpack_la_SOURCES	= unpack.c
urn_la_SOURCES		= urn.c
wrap_la_SOURCES		= wrap.c
z__la_SOURCES		= z~.c

zexy_la_SOURCES	= zexy.c z_zexy.c \
	zexy.h zexySIMD.h z_zexy.h zexyconf.h \
	0x260x260x7e.c 0x2e.c 0x3c0x7e.c 0x3d0x3d0x7e.c 0x3e0x7e.c 0x7c0x7c0x7e.c \
	a2l.c abs~.c absgn~.c atoi.c avg~.c  \
	blockmirror~.c blockshuffle~.c blockswap~.c \
	date.c demultiplex~.c demultiplex.c dfreq~.c dirac~.c drip.c envrms~.c \
	fifop.c freadln.c fwriteln.c glue.c index.c length.c lifop.c limiter~.c \
	list2int.c list2lists.c list2symbol.c lister.c listfind.c liststorage.c lpt.c \
	makesymbol.c matchbox.c mavg.c minmax.c msgfile.c multiline~.c multiplex~.c \
	multiplex.c multireceive.c niagara.c noish~.c noisi~.c operating_system.c pack~.c \
	pack.c packel.c pdf~.c prime.c quantize~.c rawprint.c regex.c relay.c repack.c \
	repeat.c route~.c sfplay.c sfrecord.c sgn~.c sigzero~.c sleepgrain.c sort.c \
	step~.c strcmp.c sum.c swap~.c symbol2list.c tabdump.c tabminmax.c tabread4~~.c \
	tabset.c tavg~.c time.c unpack~.c unpack.c urn.c wrap.c z~.c
if WINDOWS
zexy_la_SOURCES	+= winNT_portio.c
endif

