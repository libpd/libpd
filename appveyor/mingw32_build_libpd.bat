SET MSYS=C:\msys64
SET MINGW=%MSYS%\mingw32
SET PATH=%MINGW%\bin;%MSYS%\usr\bin
make -C ../ clobber
make -C ../ libpd MULTI=true
