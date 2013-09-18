;
; uninstall only (and all) installed files
; ripped from http://nsis.sourceforge.net/Uninstall_only_installed_files
;
; Part 1/2: THIS HAS TO GO DIRECTLY BEFORE THE "SECTIONS"
;
;   Instead of using SetOutPath, CreateDirectory, File, CopyFiles, Rename and
;   WriteUninstaller instructions in your sections, use ${SetOutPath},
;   ${CreateDirectory}, ${File}, ${CopyFiles}, ${Rename} and ${WriteUninstaller}
;   instead.
;   
;   When using ${SetOutPath} to create more than one upper level directory, e.g.:
;   ${SetOutPath} "$INSTDIR\dir1\dir2\dir3", you need to add entries for each lower
;   level directory for them all to be deleted:
;        ${AddItem} "$INSTDIR\dir1"
;        ${AddItem} "$INSTDIR\dir1\dir2"
;        ${SetOutPath} "$INSTDIR\dir1\dir2\dir3"

!define UninstLog "uninstall.log"
Var UninstLog
 
; Uninstall log file missing.
LangString UninstLogMissing ${LANG_ENGLISH} "${UninstLog} not found!$\r$\nUninstallation cannot proceed!"
 
; AddItem macro
!macro AddItem Path
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define AddItem "!insertmacro AddItem"
 
; File macro
!macro File FilePath FileName
 IfFileExists "$OUTDIR\${FileName}" +2
  FileWrite $UninstLog "$OUTDIR\${FileName}$\r$\n"
 File "${FilePath}${FileName}"
!macroend
!define File "!insertmacro File"
 
; CreateShortcut macro
!macro CreateShortcut FilePath FilePointer
 FileWrite $UninstLog "${FilePath}$\r$\n"
 CreateShortcut "${FilePath}" "${FilePointer}"
!macroend
!define CreateShortcut "!insertmacro CreateShortcut"
 
; Copy files macro
!macro CopyFiles SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 CopyFiles "${SourcePath}" "${DestPath}"
!macroend
!define CopyFiles "!insertmacro CopyFiles"
 
; Rename macro
!macro Rename SourcePath DestPath
 IfFileExists "${DestPath}" +2
  FileWrite $UninstLog "${DestPath}$\r$\n"
 Rename "${SourcePath}" "${DestPath}"
!macroend
!define Rename "!insertmacro Rename"
 
; CreateDirectory macro
!macro CreateDirectory Path
 CreateDirectory "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define CreateDirectory "!insertmacro CreateDirectory"
 
; SetOutPath macro
!macro SetOutPath Path
 SetOutPath "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define SetOutPath "!insertmacro SetOutPath"
 
; WriteUninstaller macro
!macro WriteUninstaller Path
 WriteUninstaller "${Path}"
 FileWrite $UninstLog "${Path}$\r$\n"
!macroend
!define WriteUninstaller "!insertmacro WriteUninstaller"
 
Section -openlogfile
 CreateDirectory "$INSTDIR"
 IfFileExists "$INSTDIR\${UninstLog}" +3
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" w
 Goto +4
  SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
  FileOpen $UninstLog "$INSTDIR\${UninstLog}" a
  FileSeek $UninstLog 0 END
SectionEnd
