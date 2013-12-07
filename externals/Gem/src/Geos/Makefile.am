#####################################################################
# Gem/Geos: objectclasses for geometric objects
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libGeos.la

libGeos_la_CXXFLAGS =
libGeos_la_LIBADD   =
libGeos_la_LDFLAGS  =

# RTE flags
libGeos_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libGeos_la_LIBADD   += @GEM_RTE_LIBS@
libGeos_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# FTGL
libGeos_la_CXXFLAGS += @GEM_LIB_FTGL_CFLAGS@
libGeos_la_LIBADD   += @GEM_LIB_FTGL_LIBS@


libGeos_la_SOURCES=  \
    circle.cpp \
    circle.h \
    colorSquare.cpp \
    colorSquare.h \
    cone.cpp \
    cone.h \
    cube.cpp \
    cube.h \
    cuboid.cpp \
    cuboid.h \
    curve3d.cpp \
    curve3d.h \
    curve.cpp \
    curve.h \
    cylinder.cpp \
    cylinder.h \
    disk.cpp \
    disk.h \
    GemSplash.cpp \
    GemSplash.h \
    imageVert.cpp \
    imageVert.h \
    mesh_line.cpp \
    mesh_line.h \
    mesh_square.cpp \
    mesh_square.h \
    model.cpp \
    model.h \
    model_loader.cpp \
    model_loader.h \
    multimodel.cpp \
    multimodel.h \
    newWave.cpp \
    newWave.h \
    polygon.cpp \
    polygon.h \
    pqtorusknots.cpp \
    pqtorusknots.h \
    primTri.cpp \
    primTri.h \
    rectangle.cpp \
    rectangle.h \
    ripple.cpp \
    ripple.h \
    rubber.cpp \
    rubber.h \
    scopeXYZ~.cpp \
    scopeXYZ~.h \
    slideSquares.cpp \
    slideSquares.h \
    sphere3d.cpp \
    sphere3d.h \
    sphere.cpp \
    sphere.h \
    square.cpp \
    square.h \
    surface3d.cpp \
    surface3d.h \
    teapot.cpp \
    teapot.h \
    text2d.cpp \
    text2d.h \
    text3d.cpp \
    text3d.h \
    textextruded.cpp \
    textextruded.h \
    textoutline.cpp \
    textoutline.h \
    torus.cpp \
    torus.h \
    trapezoid.cpp \
    trapezoid.h \
    triangle.cpp \
    triangle.h \
    tube.cpp \
    tube.h

