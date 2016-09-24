SET MSYS=C:\msys32
SET MINGW=%MSYS%\mingw32
SET PATH=%MINGW%\bin;%MSYS%\usr\bin
make clobber
make csharplib
