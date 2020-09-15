SET MSYS=C:\msys64
SET MINGW=%MSYS%\mingw64
SET PATH=%MINGW%\bin;%MSYS%\usr\bin
make clobber
:: force long long 64 bit int type as mingw64 uses 32 bit by default
make csharplib ADDITIONAL_CFLAGS='-DPD_LONGINTTYPE="long long"'