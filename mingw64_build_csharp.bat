SET MSYS=C:\msys64
SET MINGW=%MSYS%\mingw64
SET PATH=%MINGW%\bin;%MSYS%\usr\bin
make clobber
make csharplib
