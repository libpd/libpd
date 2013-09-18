# Microsoft Developer Studio Project File - Name="iemmatrix" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** NICHT BEARBEITEN **

# TARGTYPE "Win32 (x86) Dynamic-Link Library" 0x0102

CFG=IEMMATRIX - WIN32 RELEASE
!MESSAGE Dies ist kein gültiges Makefile. Zum Erstellen dieses Projekts mit NMAKE
!MESSAGE verwenden Sie den Befehl "Makefile exportieren" und führen Sie den Befehl
!MESSAGE 
!MESSAGE NMAKE /f "iemmatrix.mak".
!MESSAGE 
!MESSAGE Sie können beim Ausführen von NMAKE eine Konfiguration angeben
!MESSAGE durch Definieren des Makros CFG in der Befehlszeile. Zum Beispiel:
!MESSAGE 
!MESSAGE NMAKE /f "iemmatrix.mak" CFG="IEMMATRIX - WIN32 RELEASE"
!MESSAGE 
!MESSAGE Für die Konfiguration stehen zur Auswahl:
!MESSAGE 
!MESSAGE "iemmatrix - Win32 Release" (basierend auf  "Win32 (x86) Dynamic-Link Library")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 1
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe
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
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /D "_USRDLL" /D "IEMMATRIX_EXPORTS" /YX /FD /c
# ADD CPP /nologo /Zp16 /W3 /GX /I "C:\Programme\pd\src" /D "PD" /D "MSW" /D "__WIN32__" /D "IEMMATRIX" /FR /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /win32
# SUBTRACT MTL /mktyplib203
# ADD BASE RSC /l 0xc07 /d "NDEBUG"
# ADD RSC /l 0xc07
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /dll /machine:I386
# ADD LINK32 kernel32.lib wsock32.lib uuid.lib libc.lib oldnames.lib pd.lib /nologo /dll /machine:I386 /nodefaultlib /out:"..\iemmatrix.dll" /libpath:"../../bin" /libpath:"C:\Programme\pd\bin" /export:iemmatrix_setup
# SUBTRACT LINK32 /pdb:none
# Begin Target

# Name "iemmatrix - Win32 Release"
# Begin Group "Quellcodedateien"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\iemmatrix.c
# End Source File
# Begin Source File

SOURCE=.\iemmatrix_binops.c
# End Source File
# Begin Source File

SOURCE=.\iemmatrix_sources.c
# End Source File
# Begin Source File

SOURCE=.\iemmatrix_utility.c
# End Source File
# Begin Source File

SOURCE=.\matrix.c
# End Source File
# Begin Source File

SOURCE=.\mtx_abs.c
# End Source File
# Begin Source File

SOURCE=.\mtx_add.c
# End Source File
# Begin Source File

SOURCE=.\mtx_and.c
# End Source File
# Begin Source File

SOURCE=.\mtx_atan.c
# End Source File
# Begin Source File

SOURCE=.\mtx_bitand.c
# End Source File
# Begin Source File

SOURCE=.\mtx_bitleft.c
# End Source File
# Begin Source File

SOURCE=.\mtx_bitor.c
# End Source File
# Begin Source File

SOURCE=.\mtx_bitright.c
# End Source File
# Begin Source File

SOURCE=.\mtx_bspline.c
# End Source File
# Begin Source File

SOURCE=.\mtx_check.c
# End Source File
# Begin Source File

SOURCE=.\mtx_cholesky.c
# End Source File
# Begin Source File

SOURCE=.\mtx_col.c
# End Source File
# Begin Source File

SOURCE=.\mtx_colon.c
# End Source File
# Begin Source File

SOURCE=.\mtx_concat.c
# End Source File
# Begin Source File

SOURCE=.\mtx_conv.c
# End Source File
# Begin Source File

SOURCE=.\mtx_cos.c
# End Source File
# Begin Source File

SOURCE=.\mtx_cumsum.c
# End Source File
# Begin Source File

SOURCE=.\mtx_dbtopow.c
# End Source File
# Begin Source File

SOURCE=.\mtx_dbtorms.c
# End Source File
# Begin Source File

SOURCE=.\mtx_decay.c
# End Source File
# Begin Source File

SOURCE=.\mtx_diag.c
# End Source File
# Begin Source File

SOURCE=.\mtx_diegg.c
# End Source File
# Begin Source File

SOURCE=.\mtx_diff.c
# End Source File
# Begin Source File

SOURCE=.\mtx_dispersive_dline.c
# End Source File
# Begin Source File

SOURCE=.\mtx_distance2.c
# End Source File
# Begin Source File

SOURCE=.\mtx_egg.c
# End Source File
# Begin Source File

SOURCE=.\mtx_eig.c
# End Source File
# Begin Source File

SOURCE=.\mtx_element.c
# End Source File
# Begin Source File

SOURCE=.\mtx_eq.c
# End Source File
# Begin Source File

SOURCE=.\mtx_exp.c
# End Source File
# Begin Source File

SOURCE=.\mtx_eye.c
# End Source File
# Begin Source File

SOURCE=.\mtx_fft.c
# End Source File
# Begin Source File

SOURCE=.\mtx_fill.c
# End Source File
# Begin Source File

SOURCE=.\mtx_find.c
# End Source File
# Begin Source File

SOURCE=.\mtx_gauss.c
# End Source File
# Begin Source File

SOURCE=.\mtx_ge.c
# End Source File
# Begin Source File

SOURCE=.\mtx_gt.c
# End Source File
# Begin Source File

SOURCE=.\mtx_ifft.c
# End Source File
# Begin Source File

SOURCE=.\mtx_index.c
# End Source File
# Begin Source File

SOURCE=.\mtx_int.c
# End Source File
# Begin Source File

SOURCE=.\mtx_inverse.c
# End Source File
# Begin Source File

SOURCE=.\mtx_isequal.c
# End Source File
# Begin Source File

SOURCE=.\mtx_le.c
# End Source File
# Begin Source File

SOURCE=.\mtx_log.c
# End Source File
# Begin Source File

SOURCE=.\mtx_lt.c
# End Source File
# Begin Source File

SOURCE=.\mtx_max2.c
# End Source File
# Begin Source File

SOURCE=.\mtx_mean.c
# End Source File
# Begin Source File

SOURCE=.\mtx_min2.c
# End Source File
# Begin Source File

SOURCE=.\mtx_minmax.c
# End Source File
# Begin Source File

SOURCE=.\mtx_mul.c
# End Source File
# Begin Source File

SOURCE=.\mtx_mul~.c
# End Source File
# Begin Source File

SOURCE=.\mtx_neq.c
# End Source File
# Begin Source File

SOURCE=.\mtx_not.c
# End Source File
# Begin Source File

SOURCE=.\mtx_ones.c
# End Source File
# Begin Source File

SOURCE=.\mtx_or.c
# End Source File
# Begin Source File

SOURCE=.\mtx_pack~.c
# End Source File
# Begin Source File

SOURCE=.\mtx_pivot.c
# End Source File
# Begin Source File

SOURCE=.\mtx_pow.c
# End Source File
# Begin Source File

SOURCE=.\mtx_powtodb.c
# End Source File
# Begin Source File

SOURCE=.\mtx_print.c
# End Source File
# Begin Source File

SOURCE=.\mtx_prod.c
# End Source File
# Begin Source File

SOURCE=.\mtx_rand.c
# End Source File
# Begin Source File

SOURCE=.\mtx_repmat.c
# End Source File
# Begin Source File

SOURCE=.\mtx_resize.c
# End Source File
# Begin Source File

SOURCE=.\mtx_reverse.c
# End Source File
# Begin Source File

SOURCE=.\mtx_rfft.c
# End Source File
# Begin Source File

SOURCE=.\mtx_rifft.c
# End Source File
# Begin Source File

SOURCE=.\mtx_rmstodb.c
# End Source File
# Begin Source File

SOURCE=.\mtx_roll.c
# End Source File
# Begin Source File

SOURCE=.\mtx_row.c
# End Source File
# Begin Source File

SOURCE=.\mtx_scroll.c
# End Source File
# Begin Source File

SOURCE=.\mtx_sin.c
# End Source File
# Begin Source File

SOURCE=.\mtx_size.c
# End Source File
# Begin Source File

SOURCE=.\mtx_slice.c
# End Source File
# Begin Source File

SOURCE=.\mtx_sndfileread.c
# End Source File
# Begin Source File

SOURCE=.\mtx_sort.c
# End Source File
# Begin Source File

SOURCE=.\mtx_spherical_harmonics.c
# End Source File
# Begin Source File

SOURCE=.\mtx_sub.c
# End Source File
# Begin Source File

SOURCE=.\mtx_sum.c
# End Source File
# Begin Source File

SOURCE=.\mtx_svd.c
# End Source File
# Begin Source File

SOURCE=.\mtx_trace.c
# End Source File
# Begin Source File

SOURCE=.\mtx_transpose.c
# End Source File
# Begin Source File

SOURCE=.\mtx_unpack~.c
# End Source File
# Begin Source File

SOURCE=.\mtx_zeros.c
# End Source File
# End Group
# Begin Group "Header-Dateien"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\iemmatrix.h
# End Source File
# Begin Source File

SOURCE=.\iemmatrix_sources.h
# End Source File
# Begin Source File

SOURCE=.\mtx_binop_generic.h
# End Source File
# End Group
# Begin Group "generic"

# PROP Default_Filter ""
# End Group
# End Target
# End Project
