#####################################################################
# Gem/Gem: core infrastructure of Gem
#####################################################################

AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -I$(top_srcdir)/src
include ../check-sources.mk

DISTCLEANFILES =

noinst_LTLIBRARIES = libGem.la

libGem_la_CXXFLAGS = -DHAVE_VERSION_H
libGem_la_LIBADD   =
libGem_la_LDFLAGS  =

# RTE flags
libGem_la_CXXFLAGS += @GEM_RTE_CFLAGS@ @GEM_ARCH_CXXFLAGS@
libGem_la_LIBADD   += @GEM_RTE_LIBS@
libGem_la_LDFLAGS  += @GEM_ARCH_LDFLAGS@


# GLEW
libGem_la_CXXFLAGS += @GEM_LIB_GLEW_CFLAGS@
libGem_la_LIBADD   += @GEM_LIB_GLEW_LIBS@


## version stuff
.PHONY: FORCE

FORCE:

Version.cpp: version_current.h FORCE

version_current.h: $(srcdir)/../version.h.in FORCE
	$(SHELL) $(srcdir)/../pkgversion.sh $< > $@

DISTCLEANFILES += version_current.h
CHECKUNUSED_IGNORE = version_current.h
## /version stuff


libGem_la_includedir = $(includedir)/Gem/Gem
libGem_la_include_HEADERS = \
	ExportDef.h \
	Version.h \
	RTE.h \
	State.h \
	Cache.h \
	Exception.h \
	Dylib.h \
	Files.h \
	ContextData.h \
	Properties.h \
	Settings.h \
	Loaders.h \
	Manager.h \
	PBuffer.h \
	Event.h

libGem_la_include_HEADERS += \
	GemGL.h \
	glew.h \
	glxew.h \
	wglew.h \
	GLStack.h

libGem_la_include_HEADERS += \
	Image.h \
	ImageIO.h \
	PixConvert.h

libGem_la_include_HEADERS += \
	WorkerThread.h \
	SynchedWorkerThread.h \
	ThreadMutex.h \
	ThreadSemaphore.h

libGem_la_SOURCES=  \
	configDarwin.h \
	configLinux.h \
	configNT.h \
	GemConfig.h \
	GemGL.h \
	glew.cpp \
	glew.h \
	glxew.h \
	wglew.h \
	Cache.cpp \
	Cache.h \
	ContextData.cpp \
	ContextData.h \
	Dylib.cpp \
	Dylib.h \
	Event.cpp \
	Event.h \
	Exception.cpp \
	Exception.h \
	ExportDef.h \
	Files.cpp \
	Files.h \
	GLStack.cpp \
	GLStack.h \
	Image.cpp \
	Image.h \
	ImageLoad.cpp \
	ImageSave.cpp \
	ImageIO.h \
	PixConvert.h \
	PixConvertAltivec.cpp \
	PixConvertSSE2.cpp \
	Loaders.cpp \
	Loaders.h \
	Manager.cpp \
	Manager.h \
	PBuffer.cpp \
	PBuffer.h \
	Properties.cpp \
	Properties.h \
	RTE.h \
	Settings.cpp \
	Settings.h \
	Setup.cpp \
	State.cpp \
	State.h \
	SynchedWorkerThread.cpp \
	SynchedWorkerThread.h \
	ThreadMutex.cpp \
	ThreadMutex.h \
	ThreadSemaphore.cpp \
	ThreadSemaphore.h \
	Version.cpp \
	Version.h \
	WorkerThread.cpp \
	WorkerThread.h

