# testprogramm
#* Author: Thomas Grill t.grill [at] gmx.net
# License LGPL see LICENSE.txt
# IEM - Institute of Electronic Music and Acoustics, Graz
# Inffeldgasse 10/3, 8010 Graz, Austria
# http://iem.at 
#************************************************************/
# --- import some functions ---

import xmlrpclib
import time
import sys

# --- connect to external ---

s = xmlrpclib.ServerProxy("http://localhost:8000")

# --- load patch ----

if len(sys.argv) >= 2:
	patchname = sys.argv[1]
else:
	# defaulting
	patchname = 'xmlrpc-test.pd'

print "Loading file:",patchname

# open PD patch as file
patch=open(patchname,'r')

# read all lines of patch
lines=patch.readlines()

# function to concatenate two lines
def concat(l,b):
	if b[0] != "#":
		l[len(l)-1] = l[len(l)-1]+b
	else:
		return l.append(b)

# add all lines that don't begin with # to their predecendants
# this is necessary for objects with many parameters (e.g. IEM GUI objects)
clines = []
for l in lines:
	concat(clines,l)

# send it to the external
s.load(clines)

# close file
patch.close()

# --- send some data ---

s.send("recv1",4)
s.send("recv2",[1,2,3,"df"])

# --- receive some data ---

# bind to a PD symbol
s.bind("send3")

# get the value
print "Got: ", s.query("send3")

# unbind again (only necessary to save PD resources)
s.unbind("send3")

# --- wait a bit ---

time.sleep(2)

# --- close our PD test patch ---

s.close()
