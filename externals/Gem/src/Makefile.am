## notes:
# LIBADD vs LDADD
#   LIBADD: ADDitional LIBraries
#   LDADD : ADDitional Linker Objects
# also::  LIBADD (for other libraries) or LDADD (for final executables).


AUTOMAKE_OPTIONS = foreign
ACLOCAL_AMFLAGS = -I $(top_srcdir)/m4

# make sure -lstdc++ is kept first
AM_LIBTOOLFLAGS = --preserve-dup-deps

SUBDIRS=Gem RTE Utils Base plugins 
pkglib_LTLIBRARIES = Gem.la

## make sure we use a c++-linker
Gem_la_SOURCES = 
nodist_EXTRA_Gem_la_SOURCES = dummy.cpp

## some default flags
Gem_la_CXXFLAGS =
Gem_la_LDFLAGS  = -module -avoid-version -shared -shrext .@GEM_RTE_EXTENSION@
Gem_la_LIBADD   =


# RTE flags
Gem_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
Gem_la_LIBADD   += @GEM_RTE_LIBS@
Gem_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# openGL
Gem_la_CXXFLAGS += @GL_CFLAGS@ @GLU_CFLAGS@ @AGL_CFLAGS@ 
Gem_la_LIBADD   += @GL_LIBS@   @GLU_LIBS@   @AGL_LIBS@
Gem_la_LDFLAGS  += 
# and glew
Gem_la_CXXFLAGS += @GEM_LIB_GLEW_CFLAGS@
Gem_la_LIBADD   += @GEM_LIB_GLEW_LIBS@
Gem_la_LDFLAGS  += 


# LIBADD or LDADD for convenience libraries?
#Gem_la_LIBADD = Base/libBase.la RTE/libRTE.la @GEM_RTE_LIBS@
Gem_la_LIBADD += Gem/libGem.la RTE/libRTE.la Utils/libUtils.la Base/libBase.la plugins/libplugins.la

## Output: disabled for now
if DISABLED
SUBDIRS+=Output
endif

if TARGET_CONTROLS
SUBDIRS+=Controls
Gem_la_LIBADD += Controls/libControls.la
endif
if TARGET_GEOS
SUBDIRS+=Geos
Gem_la_LIBADD += Geos/libGeos.la
endif
if TARGET_MANIPS
SUBDIRS+=Manips
Gem_la_LIBADD += Manips/libManips.la
endif
if TARGET_NONGEOS
SUBDIRS+=Nongeos
Gem_la_LIBADD += Nongeos/libNongeos.la
endif
if TARGET_OPENGL
SUBDIRS+=openGL
Gem_la_LIBADD += openGL/libopenGL.la
endif
if TARGET_PARTICLES
SUBDIRS+=Particles
Gem_la_LIBADD += Particles/libParticles.la
endif
if TARGET_PIXES
SUBDIRS+=Pixes
Gem_la_LIBADD += Pixes/libPixes.la
endif
if TARGET_VERTEX
SUBDIRS+=Vertex
Gem_la_LIBADD += Vertex/libVertex.la
endif

SUBDIRS+=deprecated

libtool: $(LIBTOOL_DEPS)
	$(SHELL) ./config.status --recheck

## uha, this is ugly
$(abs_builddir)/.libs/Gem.@GEM_RTE_EXTENSION@: Gem.la

$(top_builddir)/Gem.@GEM_RTE_EXTENSION@: $(abs_builddir)/.libs/Gem.@GEM_RTE_EXTENSION@ 
		rm -f $@ && cd $(top_builddir) && test -e $< && $(LN_S) $< Gem.@GEM_RTE_EXTENSION@ || true

.PHONY: clean-conveniencelink

clean-conveniencelink:
	test -L $(top_builddir)/Gem.@GEM_RTE_EXTENSION@ && rm -f $(top_builddir)/Gem.@GEM_RTE_EXTENSION@ || true


all-local:: $(top_builddir)/Gem.@GEM_RTE_EXTENSION@

clean-local:: clean-conveniencelink

EXTRA_Gem_la_SOURCES = \
	pkgversion.sh \
	pkgversion.txt \
	version.h.in
