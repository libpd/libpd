==============================================================================
		IEMguts
==============================================================================

whatsit::
---------
IEMguts is an extension-library (aka: external, plugin) for 
miller.s.puckette's realtime computer-music environment "Pure data"; 
it is of no use without Pure data

IEMguts is a collection of objects that deal with the infrastructure to build 
better abstractions.
they might be of no use on their own...


danger::
--------
IEMguts often deal with non-official internals of Pure data.
These internals might change in future versions of Pd, rendering IEMguts unuseable.
Always make sure that you are using IEMguts with the same version as it was compiled with!
Else you might experience non-functional, crashing or exploding intestines.

installation::
--------------
#1> cd src/
#2> make
#3> make install

note: IEMguts depends on some internal headers of Pd.
therefore you might have to specify the full path to your Pd-sources
using the PDROOT environment variable.
something like
#2> PDROOT=/home/me/src/pd-0.41-2/ make
should do the trick
(PDROOT should point to a directory wherein there is a src/-subdirectory
containing the sources of the Pd you are running)

license::
---------
this software is released under the GNU General Public License v2 or later
you can find the full text of this license in the GnuGPL.txt file that must
be shipped with this software

authors::
---------
this software is 
copyleft 2007- by 
	IOhannes m zmoelnig <zmoelnig [at] iem [dot] at>,
	Institute of Electronic Music and Acoustics,
	University of Music and Dramatic Arts, Graz, Austria


