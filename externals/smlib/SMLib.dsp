# Microsoft Developer Studio Project File - Name="SMLib" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=SMLib - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "SMLib.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "SMLib.mak" CFG="SMLib - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "SMLib - Win32 Release" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE "SMLib - Win32 Debug" (based on "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "SMLib - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMLIB_EXPORTS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMLIB_EXPORTS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x813 /d "NDEBUG"
# ADD RSC /l 0x813 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pd.lib /nologo /dll /machine:I386

!ELSEIF  "$(CFG)" == "SMLib - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMLIB_EXPORTS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "SMLIB_EXPORTS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x813 /d "_DEBUG"
# ADD RSC /l 0x813 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib pd.lib /nologo /dll /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "SMLib - Win32 Release"
# Name "SMLib - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\bp.c
# End Source File
# Begin Source File

SOURCE=.\decimator.c
# End Source File
# Begin Source File

SOURCE=.\deltas.c
# End Source File
# Begin Source File

SOURCE=.\hip.c
# End Source File
# Begin Source File

SOURCE=.\hist.c
# End Source File
# Begin Source File

SOURCE=.\itov.c
# End Source File
# Begin Source File

SOURCE=.\lavg.c
# End Source File
# Begin Source File

SOURCE=.\lhist.c
# End Source File
# Begin Source File

SOURCE=.\lhisti.c
# End Source File
# Begin Source File

SOURCE=.\linspace.c
# End Source File
# Begin Source File

SOURCE=.\lmax.c
# End Source File
# Begin Source File

SOURCE=.\lmin.c
# End Source File
# Begin Source File

SOURCE=.\lrange.c
# End Source File
# Begin Source File

SOURCE=.\lstd.c
# End Source File
# Begin Source File

SOURCE=.\prevl.c
# End Source File
# Begin Source File

SOURCE=.\SMLib.c
# End Source File
# Begin Source File

SOURCE=.\threshold.c
# End Source File
# Begin Source File

SOURCE=.\vabs.c
# End Source File
# Begin Source File

SOURCE=.\vclip.c
# End Source File
# Begin Source File

SOURCE=.\vcog.c
# End Source File
# Begin Source File

SOURCE=.\vdbtorms.c
# End Source File
# Begin Source File

SOURCE=.\vdelta.c
# End Source File
# Begin Source File

SOURCE=.\vfmod.c
# End Source File
# Begin Source File

SOURCE=.\vftom.c
# End Source File
# Begin Source File

SOURCE=.\vlavg.c
# End Source File
# Begin Source File

SOURCE=.\vlmax.c
# End Source File
# Begin Source File

SOURCE=.\vlmin.c
# End Source File
# Begin Source File

SOURCE=.\vlrange.c
# End Source File
# Begin Source File

SOURCE=.\vmax.c
# End Source File
# Begin Source File

SOURCE=.\vmin.c
# End Source File
# Begin Source File

SOURCE=.\vmtof.c
# End Source File
# Begin Source File

SOURCE=.\vnmax.c
# End Source File
# Begin Source File

SOURCE=.\vpow.c
# End Source File
# Begin Source File

SOURCE=.\vrms.c
# End Source File
# Begin Source File

SOURCE=.\vrmstodb.c
# End Source File
# Begin Source File

SOURCE=.\vstd.c
# End Source File
# Begin Source File

SOURCE=.\vsum.c
# End Source File
# Begin Source File

SOURCE=.\vthreshold.c
# End Source File
# Begin Source File

SOURCE=.\vvconv.c
# End Source File
# Begin Source File

SOURCE=.\vvminus.c
# End Source File
# Begin Source File

SOURCE=.\vvplus.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\defines.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# Begin Source File

SOURCE=.\readme.txt
# End Source File
# End Target
# End Project
