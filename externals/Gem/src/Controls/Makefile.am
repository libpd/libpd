#####################################################################
# Gem/Controls: objectclasses that control rendering
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libControls.la

libControls_la_CXXFLAGS =
libControls_la_LIBADD   =
libControls_la_LDFLAGS  =

# RTE flags
libControls_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libControls_la_LIBADD   += @GEM_RTE_LIBS@
libControls_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libControls_la_SOURCES=  \
    gemframebuffer.cpp \
    gemframebuffer.h \
    gemhead.cpp \
    gemhead.h \
    gemkeyboard.cpp \
    gemkeyboard.h \
    gemkeyname.cpp \
    gemkeyname.h \
    gemlist.cpp \
    gemlist.h \
    gemlist_info.cpp \
    gemlist_info.h \
    gemlist_matrix.cpp \
    gemlist_matrix.h \
    gemmouse.cpp \
    gemmouse.h \
    gemreceive.cpp \
    gemreceive.h \
    gemwin.cpp \
    gemwin.h \
    render_trigger.cpp \
    render_trigger.h

