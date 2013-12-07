current: cyclone

.SUFFIXES: .dll .obj

ROOTDIR = ..\..\..
SRCDIR = $(ROOTDIR)\miXed\cyclone
BINDIR = $(ROOTDIR)\miXed\bin
SHAREDDIR = $(ROOTDIR)\miXed\shared
EXTDIR = e:\PureData\externals
PDDIR = e:\PureData\pd
VCLIBDIR = "e:\Program Files\Microsoft Visual Studio\Vc98\lib"
ZIPCOMMAND = d:\dosowe\arc\info-zip\zip

INCLUDES = /I.  /I$(SHAREDDIR) /I$(PDDIR)\src
CFLAGS = /O2 /W3 /WX /DNT /DMSW /DPD /nologo

LIBS = $(VCLIBDIR)\libc.lib \
	$(VCLIBDIR)\oldnames.lib \
	$(VCLIBDIR)\wsock32.lib \
	$(VCLIBDIR)\kernel32.lib \
	$(VCLIBDIR)\uuid.lib \
	$(PDDIR)\bin\pd.lib

!INCLUDE objects
!INCLUDE snapfiles
!INCLUDE $(SRCDIR)\build_counter

cyclone: cyclone.dll hammer.dll sickle.dll dummies.dll maxmode.dll cyclist.exe
	@cd $(ROOTDIR)
	@$(ZIPCOMMAND) cyclone-$(CYCLONE_SNAPSHOT)-dll.zip $(SNAPFILES)

cyclone.dll: $(CYCLONE_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

hammer.dll: $(HAMMER_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

sickle.dll: $(SICKLE_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

dummies.dll: $(DUMMIES_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

maxmode.dll: $(MAXMODE_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

cyclist.exe: $(SHAREDDIR)\common\binport.c \
	$(SHAREDDIR)\common\lex.c $(SHAREDDIR)\unstable\standalone.c
	-del /S $(SHAREDDIR)\common\binport.obj \
		$(SHAREDDIR)\common\lex.obj $(SHAREDDIR)\unstable\standalone.obj
	-cl $(CFLAGS) $(INCLUDES) /DMIXED_STANDALONE /o $@ \
		$(VCLIBDIR)\libc.lib $(SHAREDDIR)\common\binport.c \
		$(SHAREDDIR)\common\lex.c $(SHAREDDIR)\unstable\standalone.c
	-@move $@ $(BINDIR)
	-del /S binport.obj lex.obj standalone.obj

.c.obj: ; cl /c $(CFLAGS) $(INCLUDES) /Fo$*.obj $*.c

clean:
	for %1 in (hammer shadow sickle) \
		do del %1\*.obj
	for %1 in (hammer shadow sickle) \
		do del %1\*.exp
	for %1 in (hammer shadow sickle) \
		do del %1\*.lib
	for %1 in (hammer shadow sickle) \
		do del %1\*.dll
