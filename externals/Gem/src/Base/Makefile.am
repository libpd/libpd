#####################################################################
# Gem/Base Base-classes for Pd-objectclasses
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libBase.la

libBase_la_CXXFLAGS =
libBase_la_LIBADD   =
libBase_la_LDFLAGS  =

# RTE flags
libBase_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libBase_la_LIBADD   += @GEM_RTE_LIBS@
libBase_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

# X
libBase_la_CXXFLAGS += $(X_CFLAGS)
libBase_la_LIBADD   += $(X_PRE_LIBS) $(X_LIBS) $(X_EXTRA_LIBS)

# openGL
libBase_la_CXXFLAGS += @GLX_CFLAGS@
libBase_la_LIBADD   += @GLX_LIBS@

# GLEW
libBase_la_CXXFLAGS += @GEM_LIB_GLEW_CFLAGS@
libBase_la_LIBADD   += @GEM_LIB_GLEW_LIBS@

# FTGL
libBase_la_CXXFLAGS += @GEM_LIB_FTGL_CFLAGS@
libBase_la_LIBADD   += @GEM_LIB_FTGL_LIBS@

libBase_la_includedir = $(includedir)/Gem/Base
libBase_la_include_HEADERS= \
	CPPExtern.h \
	GemBase.h \
	GemGLBase.h \
	GemGluObj.h \
	GemPathBase.h \
	GemPixObj.h \
	GemPixDualObj.h \
	GemShape.h \
	TextBase.h

libBase_la_include_HEADERS+= \
	GemWindow.h \
	GemContext.h 

libBase_la_SOURCES=  \
    CPPExtern.cpp \
    CPPExtern.h \
    GemBase.cpp \
    GemBase.h \
    GemGLBase.h \
    GemContext.cpp \
    GemContext.h \
    GemGluObj.cpp \
    GemGluObj.h \
    GemPathBase.cpp \
    GemPathBase.h \
    GemPixDualObj.cpp \
    GemPixDualObj.h \
    GemPixObj.cpp \
    GemPixObj.h \
    GemShape.cpp \
    GemShape.h \
    GemVertex.cpp \
    GemVertex.h \
    GemWinCreate.h \
    GemWinCreateMac.cpp \
    GemWinCreateNT.cpp \
    GemWinCreateXWin.cpp \
    GemWindow.cpp \
    GemWindow.h \
    TextBase.cpp \
    TextBase.h

