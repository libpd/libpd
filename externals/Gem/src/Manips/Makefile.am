#####################################################################
# Gem/Manips: objectclasses to manipulate Geos
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libManips.la

libManips_la_CXXFLAGS =
libManips_la_LIBADD   =
libManips_la_LDFLAGS  =

# RTE flags
libManips_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libManips_la_LIBADD   += @GEM_RTE_LIBS@
libManips_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libManips_la_SOURCES=  \
    accumrotate.cpp \
    accumrotate.h \
    alpha.cpp \
    alpha.h \
    ambient.cpp \
    ambient.h \
    ambientRGB.cpp \
    ambientRGB.h \
    camera.cpp \
    camera.h \
    color.cpp \
    color.h \
    colorRGB.cpp \
    colorRGB.h \
    depth.cpp \
    depth.h \
    diffuse.cpp \
    diffuse.h \
    diffuseRGB.cpp \
    diffuseRGB.h \
    emission.cpp \
    emission.h \
    emissionRGB.cpp \
    emissionRGB.h \
    fragment_program.cpp \
    fragment_program.h \
    glsl_fragment.cpp \
    glsl_fragment.h \
    glsl_geometry.cpp \
    glsl_geometry.h \
    glsl_program.cpp \
    glsl_program.h \
    glsl_vertex.cpp \
    glsl_vertex.h \
    linear_path.cpp \
    linear_path.h \
    ortho.cpp \
    ortho.h \
    polygon_smooth.cpp \
    polygon_smooth.h \
    rotate.cpp \
    rotate.h \
    rotateXYZ.cpp \
    rotateXYZ.h \
    scale.cpp \
    scale.h \
    scaleXYZ.cpp \
    scaleXYZ.h \
    separator.cpp \
    separator.h \
    shearXY.cpp \
    shearXY.h \
    shearXZ.cpp \
    shearXZ.h \
    shearYX.cpp \
    shearYX.h \
    shearYZ.cpp \
    shearYZ.h \
    shearZX.cpp \
    shearZX.h \
    shearZY.cpp \
    shearZY.h \
    shininess.cpp \
    shininess.h \
    specular.cpp \
    specular.h \
    specularRGB.cpp \
    specularRGB.h \
    spline_path.cpp \
    spline_path.h \
    translate.cpp \
    translate.h \
    translateXYZ.cpp \
    translateXYZ.h \
    vertex_program.cpp \
    vertex_program.h

