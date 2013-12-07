current: pddp

.SUFFIXES: .dll .obj

ROOTDIR = ..\..\..
SRCDIR = $(ROOTDIR)\miXed\pddp
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

pddp: pddpboot pddplink.dll
	@cd $(ROOTDIR)
	@$(ZIPCOMMAND) pddp-$(PDDP_SNAPSHOT)-dll.zip $(SNAPFILES)

pddpboot:
	-@copy $(SRCDIR)\pddpboot.tcl $(EXTDIR)
	-@copy $(SRCDIR)\pddpclient.tcl $(EXTDIR)
	-@copy $(SRCDIR)\pddpserver.tcl $(EXTDIR)
	-@copy $(SRCDIR)\pddpboot.tcl $(BINDIR)
	-@copy $(SRCDIR)\pddpclient.tcl $(BINDIR)
	-@copy $(SRCDIR)\pddpserver.tcl $(BINDIR)

pddplink.dll: $(PDDPLINK_OBJECTS)
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
