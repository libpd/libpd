This is the readme for "Chaos PD Externals" a set of objects for PD which 
calculate various "Chaotic Attractors"; including: lorenz, rossler, henon, 
ikeda, attract1, base, base3, dejong, gingerbreadman, hopalong, latoocarfian, 
latoomutalpha, latoomutbeta, latoomutgamma, logistic, lotka_volterra, martin,
mlogistic, pickover, popcorn, quadruptwo, standardmap, strange1, tent, three_d, threeply, tinkerbell and unity.

If you have any questions/comments you can reach the co-authors at:
Ben Bogart		ben@ekran.org
Michael McGonagle	mjmogo@comcast.net

Please Note:
These programs are Copyright Ben Bogart 2002, Ben Bogart and Michael McGonagle 2003.

These programs are distributed under the terms of the GNU General Public 
License 

Chaos PD Externals are free software; you can redistribute them and/or modify
them under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or 
(at your option) any later version.

Chaos PD Externals are distributed in the hope that they will be useful, 
but WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details. 

You should have received a copy of the GNU General Public License
along with the Chaos PD Externals; if not, write to the Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

USAGE:

The package includes 1, 2 and 3 dimentional attractors. There are outlets for 
each dimention, starting from the left, followed by three outlets for attractor
data (see the help patches for details). The scale of the values vary between 
the different attractors. To run pd with the chaos externals use:

pd -lib chaos

The basic object methods are as follows:

bang:	Calculate one iteration of the attractor.
reset:	Reset to initial conditions defined by the two or three arguments.
        [reset a b c] will reset the xyz values to abc respectively.
param:  Modify the paramaters of the equation, the number of args depend
	on the attractor. (Be careful with the parameters, an attractor
	will go from stable to infinity in very few interations.)

See the example patches for other methods (parameter searching etc.)

The next release will include a script to generate your own attractor object 
for chaos from the attractor attributes. 

Have Fun.
