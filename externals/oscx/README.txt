OSC, OpenSoundControl for pd
============================
for more information on OSC see: http://cnmat.cnmat.berkeley.edu/OSC

for more information on pure-data: http://lena.ucsd.edu/~msp/ and http://www.google.com/search?q=pure-data

log:

	20050830: v0.3: (piotr@majdak.com)
						adapted to compile on Windows2000
						dumpOSC routes up to 128 branches (tested with 24)
						sendOSC doesn't crash on longer messages (tested with one argument of 120 characters)
						sendOSC # of arguments limited by the length of the message (tested with 110 messages)
	          tested on Windows 2000 ONLY!

  20040409: changed build setup to suit externals build system
            single object objects, no lib

  20030531: added OSCroute /* (route everything) hard-fix

  20030527: added sending to broadcast address capability to htmsocket

  20020908: 0.16-4:
  	    added non-match / unmatched outlet to OSCroute
  	    updated doc/OSCroute-help.pd including a new chapter
	    about patternmatching.

  20020901: ca., refixed MAXPDARG vs. MAX_ARGS, causing crash when sending
            messages with more than 4 arguments

  20020417: 0.16-2:
            more changes by raf + jdl (send with no argument fix, send / fix,
            ...)

  20020416: added bundle stuff to sendOSC

  200204: 0.15b1:
          windowified version and implied linux enhancements
          by raf@interaccess.com
	  for now get it at http://207.208.254.239/pd/win32_osc_02.zip
	  most importantly: enhanced connect->disconnect-connect behaviour
          (the win modifications to libOSC are still missing in _this_
	   package but coming ..)


  200203: 0-0.1b1: all the rest
	  ost_at_test.at + i22_at_test.at, 2000-2002
      	  modified to compile as pd externel




INSTALL:
 (linux)

tar zxvf OSCx.tgz
cd OSCx
cat README
cd libOSC && make
cd ../OSC && "adjust makefile" && make OSC && make install
cd ../..
pd -lib OSC OSCx/doc/OSC-help.pd

 PITFALLS:
make sure you compile libOSC before OSC objects
maybe adjust include path so pd include files will be found


 (windo$)

unzip and put .dll file in a pd-searched folder.


TYPETAGS:
supported and on by default. can be swtiched off with the "typetags 0"
message and on with 1.


TODO
====
-timetags: output timetag when receiving a bundle for scheduling
-TCP mode
-address space integration with pd patch/subpatch/receive hierarchy ?

-pd object hierarchy extract and automatic address construction
 a la [/hostname]/pd/patchname/subpatch/test ?

-dynamic space allocation for message buffers.


changelog:

20020903: refixed MAXPDARG vs. MAX_ARGS bug causind sendOSC to crash
          with msgs longer than 5 argmuents. ?

20020305: -typetags in send and receive
           sendOSC by default now send typetagged msgs
	   and dumOSC properly reads and outputs them.
	   
prior:

      -added OSCroute with source adapt from max object.
      -fixed shared htmsock bug
      -added sendtyped separately earlier and lost it again
	  
   

--
jdl at xdv.org
http://barely.a.live.fm/pd/OSC
http://cvs.sourceforge.net/cgi-bin/viewcvs.cgi/pure-data/externals/OSCx/
windows version:
raf at interaccess.com, http://207.208.254.239/pd/win32_osc_02.zip
OSC protocol inventory for OSC4PD

sendOSC, dumpOSC, OSCroute
==========================
  INCLUDES
    * typed and untyped packing of OSC messages
    * #bundle packing
    * transmission of OSC messages via UDP socket

    * receive OSC messages on UDP socket
    * unpacking of typed and untyped OSC messages
    * #bundle unpacking

    * static address resolution
    * pattern matching

  OMITS
    * working with timetags
    * all advanced protocol features of documentation, typesigs, etc
    * connection oriented communication
