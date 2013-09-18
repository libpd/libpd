current: nt


# TARGETS += stk

VERSION = \"0.16\"

.SUFFIXES: .dll .obj
# ----------------------- NT ----------------------------

NTOBJECTS = *.obj
NTDLLS = *.dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo

PDNTINCLUDE = /I. /I..\..\pd\src 
	
ProgramFiles = C:\Program Files
PDNTLDIR = "$(ProgramFiles)\Microsoft Visual Studio\Vc98\lib"
#PDNTLDIR = "C:\Programme\Msdev\Vc98\lib"

PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\wsock32.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\uuid.lib \
	..\..\pd\bin\pd.lib 

nt: $(NTOBJECTS) 
	-link /dll  $(PDNTLIB) fofsynth~.obj /export:fofsynth_tilde_setup
	-link /dll  $(PDNTLIB) pvocfreq.obj /export:pvocfreq_setup
	-link /dll  $(PDNTLIB) fasor~.obj /export:fasor_tilde_setup


clean:
	del *.obj 
	del *.dll


.c.obj:
	-cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c

.obj.dll:


	

