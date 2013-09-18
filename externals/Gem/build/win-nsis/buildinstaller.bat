set NSISDIR=%ProgramFiles%\NSIS\
set GEMNSIS=%CD%

cd %NSISDIR%

set PROD=/DPRODUCT_VERSION=%1
if "%1"=="" set PROD="/DBLABLA"

makensis %PROD% %GEMNSIS%\gem.nsi

cd %GEMNSIS%
