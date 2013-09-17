@echo off

set RUNTESTS_TXT=runtests.txt
set RUNTESTS_LOG=runtests.log

set LIBFLAGS=-lib ../zexy -path ../abs/

echo %RUNTESTS_LOG% 


..\..\..\pd\bin\pd %LIBFLAGS% -nogui runtests_nogui.pd > %RUNTESTS_LOG% 2>&1

