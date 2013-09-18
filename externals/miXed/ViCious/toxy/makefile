current: toxy

.SUFFIXES: .dll .obj

ROOTDIR = ..\..\..
SRCDIR = $(ROOTDIR)\miXed\toxy
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

toxy: tot.dll tow.dll widget.dll
	@cd $(ROOTDIR)
	@$(ZIPCOMMAND) toxy-$(TOXY_SNAPSHOT)-dll.zip $(SNAPFILES)

tot.dll: $(TOT_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

tow.dll: $(TOW_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

widget.dll: $(WIDGET_OBJECTS)
	-link /dll /out:$@ $(LIBS) $** /export:$(@:.dll=_setup)
	-@copy $@ $(EXTDIR)
	-@move $@ $(BINDIR)
	-del $(@:.dll=.exp)
	-del $(@:.dll=.lib)

.c.obj: ; cl /c $(CFLAGS) $(INCLUDES) /Fo$*.obj $*.c

clean:
	del *.obj
	del *.exp
	del *.lib
	del *.dll
