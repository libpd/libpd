#####################################################################
# Gem/Utils various utilities
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libUtils.la

libUtils_la_CXXFLAGS =
libUtils_la_LIBADD   =
libUtils_la_LDFLAGS  =

# RTE flags
libUtils_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libUtils_la_LIBADD   += @GEM_RTE_LIBS@
libUtils_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@


libUtils_la_includedir = $(includedir)/Gem/Utils
libUtils_la_include_HEADERS = \
	any.h \
	Functions.h \
	GLUtil.h \
	GemMath.h \
	Matrix.h \
	PixPete.h \
	SIMD.h \
	Vector.h 


libUtils_la_SOURCES=  \
	any.h \
	Functions.cpp \
	Functions.h \
	GLUtil.cpp \
	GLUtil_generated.h \
	GLUtil.h \
	GemMath.h \
	Matrix.cpp \
	Matrix.h \
	PixPete.h \
	SIMD.cpp \
	SIMD.h \
	Vector.cpp \
	Vector.h
