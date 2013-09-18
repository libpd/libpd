pdlua -- a Lua embedding for Pd
Copyright (C) 2007,2008,2009 Claude Heiland-Allen <claudiusmaximus@goto10.org>


This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.


Lua 5.1 is highly recommended, other versions are untested and 
unsupported - use at your own risk.


Compilation Instructions:

1. edit Makefile.static to configure your PLATFORM and PDINCLUDE
2. run "make -f Makefile.static"
3. use it like "pd -path src -lib lua"

Alternatively, for the brave:

aclocal
autoheader
automake -a -c
autoconf
./configure
make
make install


Additional notes for mingw (thanks to David Shimamomto):

Copy the following files to the same directory as 'Makefile.static':
1. lua-5.1.3.tar.gz
2. m_pd.h
3. pd.dll

Edit Makefile.static to have "PDINCLUDE = -I./" (without quotes).

To install, copy the following files to 'extra':
1. src/lua.dll
2. src/pd.lua
