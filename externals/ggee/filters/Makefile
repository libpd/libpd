current: nt


# TARGETS += stk

VERSION = \"0.24\"

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
	-link /dll  $(PDNTLIB) bandpass.obj /export:bandpass_setup
	-link /dll  $(PDNTLIB) highpass.obj /export:highpass_setup
	-link /dll  $(PDNTLIB) highshelf.obj /export:highshelf_setup
	-link /dll  $(PDNTLIB) hlshelf.obj /export:hlshelf_setup
	-link /dll  $(PDNTLIB) lowpass.obj /export:lowpass_setup
	-link /dll  $(PDNTLIB) notch.obj /export:notch_setup
	-link /dll  $(PDNTLIB) equalizer.obj /export:equalizer_setup


clean:
	del *.obj 
	del *.dll


.c.obj:
	-cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c

.obj.dll:


	

