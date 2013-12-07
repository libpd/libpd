build-instructions for zexy
===========================


autoconf/make
=============
this is the preferred way to build zexy, if your system supports autoconf/make. 
such systems include:
- linux / gcc
- os-x / gcc
- windows / mingw
- windows / cygwin
- freebsd
- ...

just go into the zexy/src/ directory, and run
 % make
or alternatively:
 % aclocal
 % autoconf
 % ./configure
 % make

use
 % ./configure --help
to see flags you can pass to configure to get special builds




Microsoft Visual Studio
=======================
use the provided project-files in the directories
win-nmake: for nmake based builds
win-vs6: for Microsoft VisualStudio 6
win-vs2003: for Microsoft VisualStudio .NET 2003
