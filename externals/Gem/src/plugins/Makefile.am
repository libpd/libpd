#####################################################################
# Gem/plugins: interfaces for various plugins
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

noinst_LTLIBRARIES = libplugins.la

libplugins_la_CXXFLAGS =
libplugins_la_LIBADD   =
libplugins_la_LDFLAGS  =

# RTE flags
libplugins_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libplugins_la_LIBADD   += @GEM_RTE_LIBS@
libplugins_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@

libplugins_la_SOURCES= \
        PluginFactory.cpp \
        PluginFactory.h \
        PluginFactoryTimple.h \
        imageloader.cpp \
        imageloader.h \
        imageloaderBase.cpp \
        imageloaderBase.h \
        imagesaver.cpp \
        imagesaver.h \
        imagesaverBase.cpp \
        imagesaverBase.h \
        imageBase.cpp \
        imageBase.h \
        film.cpp \
        film.h \
        filmBase.cpp \
        filmBase.h \
        record.cpp \
        record.h \
        recordBase.cpp \
        recordBase.h \
        video.cpp \
        video.h \
        videoBase.cpp \
        videoBase.h

libplugins_la_includedir = $(includedir)/Gem/plugins
libplugins_la_include_HEADERS = \
	PluginFactory.h \
	PluginFactoryTimple.h \
	film.h \
	imageloader.h \
	imagesaver.h \
	record.h \
        video.h
