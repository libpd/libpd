set _=%CD%

set NSISDIR=..\win-nsis\

xcopy /y Gem.dll %NSISDIR%\
cd %NSISDIR%

buildinstaller.bat %1

cd %_%