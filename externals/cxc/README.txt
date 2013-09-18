--------------------------
  cxc pd eternals library
  powered by  zt0ln d4ta
--------------------------

USE AT YOUR OWN RISK!
NO WARRANTY WHATSOEVER!
THIS IS GPL SOFTWARE.
see gpl.txt in this directory or http://www.gnu.org/licenses/gpl.txt
etc blah blah ...

install:
./configure --prefix=/path/to

then type
$ make
$ make install

(install cxc.pd_linux in $PREFIX/lib/pd/externs
and documentation     in $PREFIX/lib/pd/doc/5.refernce


included objects:

ixprint:
print data on console without prefix (needed for ascwave)

binshift:
binary shift objects (<<,>>)

ascseq:
ascii-sequencer: input anything, which is output again sequentially
character by character with a given delay

ascwave:
print funny ascii constructions on console

bfilt alias bangfilter:
re-output every argumen-th event
useful for modulo-sequencers
internal: modulo x -> sel 0
 
bfilt2:
features internal counter, output just bangs

counter:
cloned out of markex so i dont need to load gem to have these

reson:
same as above

cxc_prepend:
prepend stuff with another symbol

cxc_split:
split incoming string at specified delimiter

utime:
output seconds since epoch and microsecond fraction

random1, random_fl, random_icg, random_tw, dist_normal:
various PRNG algorithms from http://remus.rutgers.edu/~rhoads/Code

random1~, random_fl~, random_icg~:
signal version of above algorithms

ENV:
get and set environment variables and certain defines like RAND_MAX

proc:
get stuff out of the linux proc directory (so far: cpuinfo, loadavg,
version, uptime)

delta~:
emit distance to last sample as signal.

cx.mean, cx.avgdev, cx.stddev:
calculate mean, standard and average deviation of a signal in an array

----------------------------------------------------------------------

x_connective*.diffs:
patch for pd-src to make receives have inlets and set method.


see reference folder for objects in action


jdl@xdv.org, 20030301
------------------------------
references see REFERENCES file
bla
