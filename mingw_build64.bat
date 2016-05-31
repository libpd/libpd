SET MINGW=C:\MinGW
SET MSYS=C:\MinGW\msys\1.0
SET MINGW64=C:\MinGW\mingw64
SET PATH=%MINGW64%\bin;%MINGW%\bin;%MSYS%\bin
make clean
make csharplib
cp libs/libpdcsharp.dll csharp/bin/Debug/