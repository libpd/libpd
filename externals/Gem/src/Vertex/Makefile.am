#####################################################################
# Gem/Vertex: objectclasses for handling VertexData
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libVertex.la

libVertex_la_CXXFLAGS =
libVertex_la_LIBADD   =
libVertex_la_LDFLAGS  =

# RTE flags
libVertex_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libVertex_la_LIBADD   += @GEM_RTE_LIBS@
libVertex_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# Dependencies
## none

libVertex_la_SOURCES=  \
    glVBO_ext.h \
    vertex_add.cpp \
    vertex_add.h \
    vertex_combine.cpp \
    vertex_combine.h \
    vertex_draw.cpp \
    vertex_draw.h \
    vertex_grid.cpp \
    vertex_grid.h \
    vertex_info.cpp \
    vertex_info.h \
    vertex_model.cpp \
    vertex_model.h \
    vertex_mul.cpp \
    vertex_mul.h \
    vertex_offset.cpp \
    vertex_offset.h \
    vertex_quad.cpp \
    vertex_quad.h \
    vertex_scale.cpp \
    vertex_scale.h \
    vertex_set.cpp \
    vertex_set.h \
    vertex_tabread.cpp \
    vertex_tabread.h

