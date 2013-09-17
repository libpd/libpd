AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = src abs reference
SUBDIRS += tests build

zexydir=$(pkglibdir)
dist_zexy_DATA = \
	AUTHORS \
	README.txt \
	INSTALL \
	LICENSE.txt \
	ChangeLog \
	zexy-meta.pd

LIBTOOL_DEPS = @LIBTOOL_DEPS@
libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status libtool
