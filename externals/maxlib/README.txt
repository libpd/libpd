maxlib - music analysis extensions library
copyright (c) 2002-2003 by Olaf Matthes

maxlib is a library of non-tilde externals for pd (by Miller Puckette).

The objects can be very useful to analyse any musical performance. Some
of the objects are 'borrowed' from Max (they are not ported but
rewritten for Pd - cheap immitations).
maxlib has recently been extended by objects of more general use and some 
which can be use for composition purposes.

To compile maxlib on win32 (using VC++ 6.0) just type "nmake pd_nt" or use
the MS VC++ project provided. On Linux simply do "make pd_linux" and "make 
install". 
You have to modify the makefile to make it point to your m_ph.h !!! 

To use maxlib place the file maxlib.dll for win32 or maxlib.pd_linux 
in a directory of your choise and start pd with '-lib path/to/maxlib' flag. 

On windows you can run install.bat to copy all files to the apropiate places.
This assumes that you have pd installed in c:\pd\ ! The maxlib directory will
then be c:\pd\externs\maxlib\


This software is published under GPL terms, see file LICENSE.

This is software with ABSOLUTELY NO WARRANTY.
Use it at your OWN RISK. It's possible to damage e.g. hardware or your hearing
due to a bug or for other reasons. 

*****************************************************************************

included objects: see http://www.akustische-kunst.org/puredata/maxlib/

Latest version can be found at:
http://www.akustische-kunst.org/puredata/maxlib/

Please report any bugs to olaf.matthes@gmx.de!
