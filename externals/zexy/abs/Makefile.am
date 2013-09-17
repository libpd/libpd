AUTOMAKE_OPTIONS = foreign

SUFFIXES = .pd

zexydir=$(pkglibdir)

dist_zexy_DATA= \
	0x260x260x7e.pd \
	0x3c0x7e-help.pd \
	0x3c0x7e.pd \
	0x3d0x3d0x7e.pd \
	0x3e0x7e-help.pd \
	0x3e0x7e.pd \
	0x7c0x7c0x7e-help.pd \
	0x7c0x7c0x7e.pd \
	a2l.pd \
	absgn~-help.pd \
	absgn~.pd \
	abs~-help.pd \
	abs~.pd \
	cart2pol-help.pd \
	cart2pol.pd \
	cart2sph-help.pd \
	cart2sph.pd \
	deg2rad-help.pd \
	deg2rad.pd \
	==~-help.pd \
	&&~-help.pd \
	length.pd \
	list2int.pd \
	mavg.pd \
	mean-help.pd \
	mean.pd \
	minmax.pd \
	nop~-help.pd \
	nop-help.pd \
	nop~.pd \
	nop.pd \
	pol2cart-help.pd \
	pol2cart.pd \
	pol2sph-help.pd \
	pol2sph.pd \
	polyfun-help.pd \
	polyfun.pd \
	rad2deg-help.pd \
	rad2deg.pd \
	repack.pd \
	segregate-help.pd \
	segregate.pd \
	sgn~-help.pd \
	sgn~.pd \
	sph2cart-help.pd \
	sph2cart.pd \
	sph2pol-help.pd \
	sph2pol.pd \
	sum.pd \
	tabset.pd \
	uniqsym-help.pd \
	uniqsym.pd
#############
# symlinks for aliases
zexyaliasdir=$(pkglibdir)

clean-local: clean_zexyalias
.PHONY: clean_zexyalias

dist_zexyalias_DATA = any2list.pd l2i.pd

# create aliases
$(dist_zexyalias_DATA):
	test -e $< && $(LN_S) $< $@

# delete aliases
clean_zexyalias:
	-rm -f $(dist_zexyalias_DATA)

# define aliases
any2list.pd: a2l.pd
l2i.pd: list2int.pd
