# Microsoft Developer Studio Project File - Name="maxlib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=maxlib - WIN32 RELEASE
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "maxlib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "maxlib.mak" CFG="maxlib - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "maxlib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "maxlib - Win32 Intel" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=xicl6.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "maxlib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir "obj\"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "maxlib_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /I "include" /D "WIN32" /D "NT" /D "_WINDOWS" /D "MAXLIB" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib wsock32.lib user32.lib uuid.lib libc.lib oldnames.lib pthreadVC.lib c:\pd\bin\pd.lib /nologo /dll /machine:I386 /nodefaultlib /libpath:"../../bin" /export:maxlib_setup
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "maxlib - Win32 Intel"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "maxlib___Win32_Intel"
# PROP BASE Intermediate_Dir "maxlib___Win32_Intel"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir ""
# PROP Intermediate_Dir ""
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /I "c:\pd\src" /D "WIN32" /D "NT" /D "_WINDOWS" /D "MAXLIB" /YX /FD /c
# ADD CPP /nologo /MT /W3 /I "include" /D "WIN32" /D "NT" /D "_WINDOWS" /D "IA32" /D "MAXLIB" /Fp"obj\maxlib.pch" /YX /Fo"obj\\" /FD /O3 /G7 /QaxW /c
# ADD BASE MTL /nologo /win32
# SUBTRACT BASE MTL /mktyplib203
# ADD MTL /nologo /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0xc07
# ADD RSC /l 0xc07
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=xilink6.exe
# ADD BASE LINK32 kernel32.lib wsock32.lib user32.lib uuid.lib libc.lib oldnames.lib pthreadVC.lib c:\pd\bin\pd.lib /nologo /dll /machine:I386 /nodefaultlib /libpath:"../../bin" /export:maxlib_setup
# SUBTRACT BASE LINK32 /pdb:none
# ADD LINK32 kernel32.lib wsock32.lib user32.lib uuid.lib libc.lib oldnames.lib pthreadVC.lib libm.lib libirc.lib svml_disp.lib c:\pd\bin\pd.lib /nologo /dll /machine:I386 /nodefaultlib /libpath:"../../bin" /export:maxlib_setup
# SUBTRACT LINK32 /pdb:none

!ENDIF 

# Begin Target

# Name "maxlib - Win32 Release"
# Name "maxlib - Win32 Intel"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\src\arbran.c
# End Source File
# Begin Source File

SOURCE=.\src\average.c
# End Source File
# Begin Source File

SOURCE=.\src\beat.c
# End Source File
# Begin Source File

SOURCE=.\src\beta.c
# End Source File
# Begin Source File

SOURCE=.\src\bilex.c
# End Source File
# Begin Source File

SOURCE=.\src\borax.c
# End Source File
# Begin Source File

SOURCE=.\src\cauchy.c
# End Source File
# Begin Source File

SOURCE=.\src\chord.c
# End Source File
# Begin Source File

SOURCE=.\src\delta.c
# End Source File
# Begin Source File

SOURCE=.\src\dist.c
# End Source File
# Begin Source File

SOURCE=.\src\divide.c
# End Source File
# Begin Source File

SOURCE=.\src\divmod.c
# End Source File
# Begin Source File

SOURCE=.\src\edge.c
# End Source File
# Begin Source File

SOURCE=.\src\expo.c
# End Source File
# Begin Source File

SOURCE=.\src\fifo.c
# End Source File
# Begin Source File

SOURCE=.\src\gauss.c
# End Source File
# Begin Source File

SOURCE=.\src\gestalt.c
# End Source File
# Begin Source File

SOURCE=.\src\history.c
# End Source File
# Begin Source File

SOURCE=.\src\ignore.c
# End Source File
# Begin Source File

SOURCE=.\src\iso.c
# End Source File
# Begin Source File

SOURCE=.\src\lifo.c
# End Source File
# Begin Source File

SOURCE=.\src\limit.c
# End Source File
# Begin Source File

SOURCE=.\src\linear.c
# End Source File
# Begin Source File

SOURCE=.\src\listfunnel.c
# End Source File
# Begin Source File

SOURCE=.\src\match.c
# End Source File
# Begin Source File

SOURCE=.\maxlib.c
# End Source File
# Begin Source File

SOURCE=.\src\minus.c
# End Source File
# Begin Source File

SOURCE=.\src\mlife.c
# End Source File
# Begin Source File

SOURCE=.\src\multi.c
# End Source File
# Begin Source File

SOURCE=.\src\netclient.c
# End Source File
# Begin Source File

SOURCE=.\src\netdist.c
# End Source File
# Begin Source File

SOURCE=.\src\netrec.c
# End Source File
# Begin Source File

SOURCE=.\src\netserver.c
# End Source File
# Begin Source File

SOURCE=.\src\nroute.c
# End Source File
# Begin Source File

SOURCE=.\src\pitch.c
# End Source File
# Begin Source File

SOURCE=.\src\plus.c
# End Source File
# Begin Source File

SOURCE=.\src\poisson.c
# End Source File
# Begin Source File

SOURCE=.\src\pong.c
# End Source File
# Begin Source File

SOURCE=.\src\pulse.c
# End Source File
# Begin Source File

SOURCE=.\src\remote.c
# End Source File
# Begin Source File

SOURCE=.\src\rhythm.c
# End Source File
# Begin Source File

SOURCE=.\src\scale.c
# End Source File
# Begin Source File

SOURCE=.\src\score.c
# End Source File
# Begin Source File

SOURCE=.\src\speedlim.c
# End Source File
# Begin Source File

SOURCE=.\src\step.c
# End Source File
# Begin Source File

SOURCE=.\src\subst.c
# End Source File
# Begin Source File

SOURCE=.\src\temperature.c
# End Source File
# Begin Source File

SOURCE=.\src\tilt.c
# End Source File
# Begin Source File

SOURCE=.\src\triang.c
# End Source File
# Begin Source File

SOURCE=.\src\velocity.c
# End Source File
# Begin Source File

SOURCE=.\src\weibull.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\include\m_imp.h
# End Source File
# Begin Source File

SOURCE=.\include\m_pd.h
# End Source File
# End Group
# End Target
# End Project
