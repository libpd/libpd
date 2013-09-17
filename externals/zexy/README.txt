==============================================================================
the zexy external
==============================================================================

outline of this file::
==============================================================================
 +  general
 +  installation
   +  linux, irix, OSX,... (autoconf)
   +  w32
 +  using
 +  license


general::
==============================================================================
the zexy external is a collection of externals for miller.s.puckette's 
realtime-computermusic-environment called "puredata" (or abbreviated "pd")
this zexy external will be of no use, if you don't have a running version of 
pd on your system.
check out for http://pd.iem.at to learn more about pd and how to get it 

note: the zexy external is published under the Gnu General Public License 
that is included (GnuGPL.txt). some parts of the code are taken directly 
from the pd source-code, they, of course, fall under the license pd is 
published under.



installation::
==============================================================================

linux, irix, osx, mingw,... :
------------------------------------------------------------------------------
see INSTALL for more detailed instructions

#0> ./autogen.sh
#1> ./configure
#2> make
#3> make install

installation directory:
by defaultm zexy will install into /usr/local/lib/pd/extra/zexy
the path can be changed via the "--prefix", or "--libdir"
e.g. "./configure --prefix=/usr" -> /usr/lib/pd/extra/zexy
e.g. "./configure --libdir=/tmp/foo" -> /tmp/foo/zexy

puredata headers:
zexy needs to find the Pd headers (and Pd.lib on some systems) during the build
process.
if you have installed the headers in a non-standard location, you can specify
them with the "--with-pd" option:
"./configure --with-pd=/usr/include/pd" will add /usr/include/pd to the INCLUDE
path.
"./configure --with-pd=/home/me/src/Pd-0.43.1" can be used to add
/home/me/src/Pd-0.43.1/src to the INCLUDEs and /home/me/src/Pd-0.43.1/bin to the
library search path

custom external extension:
zexy does it's best to determine the correct external extension for your system.
e.g. it will use "dll" on w32, or "pd_linux" on linux.
if - for whatever obscure reasons - you want to force the extension to certain
value, you can use the "--with-extension" flag:
"./configure --with-extension=l_ia64" will use "l_ia64" for the resulting
binaries

SSE2 (SIMD):
by default zexy is compiled without SIMD optimization (recently there have been
reports about crashes, when SSE2 was enabled; until this is fixed, the default
is to use the safe fallback)
if you want to enable SSE2 optimization, configure with
"./configure --enable-simd=SSE2"

multi-object vs single-object libraries:
by default, zexy builds a single library "zexy" that contains all objects.
if - for some obscure reason - you insist on having a lot of small libraries
each containing a single object, you can enable this by using the
"--disable-library" flag

parallel port support:
if you don't want the parallel-port object [lpt] you can disable it with
	"--disable-lpt"
 (e.g.: because you don't have a parallel-port)


fat (multiarch) binaries:
for building multi-arch binaries (currently only supported on OSX), specify the
wanted architectures in the "--enable-fat-binary" flag
e.g. "./configure --enable-fat-binary=i386,ppc --with-extension=d_fat"

win32 :
------------------------------------------------------------------------------

to compile: 
 + w/ MSVC use the build project found in build/win-vs*/
 OR
 + with GCC configure your pd path, eg:
	#> ./configure --prefix=/c/program/pd; make; make install
 OR
 + cross-compilation for windows on linux using mingw (assumes that the 
   crosscompiler is "i586-mingw32msvc-cc")
	#> ./configure --host=i586-mingw32msvc --with-extension=dll \
	   --disable-PIC --with-pd=/path/to/win/pd/
	#> make CFLAGS="-fno-unit-at-a-time"
     notes: configure tries to set the CFLAGS to "-g -O2" if the compiler
            accepts this; however, this optimization sometimes generates 
	    binaries that cannot be loaded by pd; it seems that disabling
	    the "unit-at-a-time" optimization (which gets enabled by "-O2")
	    is the cause of this problem. turning it off might help

making pd run with the zexy external::
==============================================================================
make sure, that pd will be looking at this location 
(add "-path <mypath>/pd/externs" either to your .pdrc or each time 
you execute pd)
make sure, that you somehow load the zexy external (either add "-lib zexy" 
(if you advised pd somehow to look at the correct place) 
or "-lib <myzexypath>/zexy" to your startup-script (.pdrc or whatever) 
or load it via the object "zexy" at runtime

license::
==============================================================================
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program.  If not, see <http://www.gnu.org/licenses/>.
