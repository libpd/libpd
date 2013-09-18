pix_artoolkit - ARToolKit support for Gem
=========================================
code by Shigeyuki Hirai


build instructions:

linux:
% aclocal -I /path/to/Gem/src/m4
% autoconf
% ./configure --with-artoolkit-includes=/path/to/artoolkit/include/ --with-artoolkit-libs=/path/to/artoolkit/lib/
% make


w32:
when using VisualStudio, you can use the project file "pix_artoolkit.sln"

