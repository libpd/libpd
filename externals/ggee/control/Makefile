current: nt


# TARGETS += stk

VERSION = \"0.16\"

.SUFFIXES: .dll .obj
# ----------------------- NT ----------------------------

NTOBJECTS = *.obj
NTDLLS = *.dll

PDNTCFLAGS = /W3 /WX /DNT /DPD /nologo

PDNTINCLUDE = /I. /I..\..\pd\src 
	
ProgramFiles = c:\Program Files
PDNTLDIR = "$(ProgramFiles)\Microsoft Visual Studio\Vc98\lib"

PDNTLIB = $(PDNTLDIR)\libc.lib \
	$(PDNTLDIR)\oldnames.lib \
	$(PDNTLDIR)\wsock32.lib \
	$(PDNTLDIR)\kernel32.lib \
	$(PDNTLDIR)\uuid.lib \
	..\..\pd\bin\pd.lib 

nt: $(NTOBJECTS) 
	-link /dll  $(PDNTLIB) constant.obj /export:constant_setup
	-link /dll  $(PDNTLIB) inv.obj /export:inv_setup
	-link /dll  $(PDNTLIB) qread.obj /export:qread_setup
	-link /dll  $(PDNTLIB) rl.obj /export:rl_setup
	-link /dll  $(PDNTLIB) rtout.obj /export:rtout_setup
	-link /dll  $(PDNTLIB) unserialize.obj /export:unserialize_setup
	-link /dll  $(PDNTLIB) serialize.obj /export:serialize_setup
	-link /dll  $(PDNTLIB) sinh.obj /export:sinh_setup
	-link /dll  $(PDNTLIB) sl.obj /export:sl_setup
	-link /dll  $(PDNTLIB) stripdir.obj /export:stripdir_setup
	-link /dll  $(PDNTLIB) unwonk.obj /export:unwonk_setup

clean:
	del *.obj 
	del *.dll


.c.obj:
	-cl $(PDNTCFLAGS) $(PDNTINCLUDE) /c $*.c

.obj.dll:


	

