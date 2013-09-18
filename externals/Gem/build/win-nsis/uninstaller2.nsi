;
; uninstall only (and all) installed files
; ripped from http://nsis.sourceforge.net/Uninstall_only_installed_files
;
; Part 2/2: THIS HAS TO GO DIRECTLY AFTER THE "SECTIONS"
;
; LATER: allow the user to proceed uninstallation even without an uninstall.log
;  (e.g. by deleting the entire tree - at the users own risk!)



Section -closelogfile
 FileClose $UninstLog
 SetFileAttributes "$INSTDIR\${UninstLog}" READONLY|SYSTEM|HIDDEN
SectionEnd
 
Section Uninstall
 
 ; Can't uninstall if uninstall log is missing!
 IfFileExists "$INSTDIR\${UninstLog}" +3
  MessageBox MB_OK|MB_ICONSTOP "$(UninstLogMissing)"
   Abort
 
 Push $R0
 Push $R1
 Push $R2
 SetFileAttributes "$INSTDIR\${UninstLog}" NORMAL
 FileOpen $UninstLog "$INSTDIR\${UninstLog}" r
 StrCpy $R1 -1
 
 GetLineCount:
  ClearErrors
  FileRead $UninstLog $R0
  IntOp $R1 $R1 + 1
  StrCpy $R0 $R0 -2
  Push $R0   
  IfErrors 0 GetLineCount
 
 Pop $R0
 
 LoopRead:
  StrCmp $R1 0 LoopDone
  Pop $R0
 
  IfFileExists "$R0\*.*" 0 +3
   RMDir $R0  #is dir
  Goto +3
  IfFileExists $R0 0 +2
   Delete $R0 #is file
 
  IntOp $R1 $R1 - 1
  Goto LoopRead
 LoopDone:
 FileClose $UninstLog
 Delete "$INSTDIR\${UninstLog}"
 RMDir "$INSTDIR"
 Pop $R2
 Pop $R1
 Pop $R0
SectionEnd
