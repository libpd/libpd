current: nt


# TARGETS += stk

VERSION = \"0.16\"

.SUFFIXES: .dll .obj
# ----------------------- NT ----------------------------

NTOBJECTS = *.obj
NTDLLS = *.dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo

PDNTINCLUDE = /I. /I..\..\pd\src  /I../include

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
	-link /dll  $(PDNTLIB) envgen.obj /export:envgen_setup
	-link /dll  $(PDNTLIB) slider.obj /export:slider_setup
	-link /dll  $(PDNTLIB) state.obj /export:state_setup
	-link /dll  $(PDNTLIB) ticker.obj /export:ticker_setup
	-link /dll  $(PDNTLIB) toddle.obj /export:toddle_setup
	-copy slider.dll hslider.dll

clean:
	del *.obj 
	del *.dll


.c.obj:
	-cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c

.obj.dll:


	

