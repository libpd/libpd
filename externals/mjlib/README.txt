mjLib 

by 
mark williamson 
mailto:mark@junklight.com
http://www.junklight.com

The code is free for anyone to use under the GNU GPL.  But if you use it, 
please mention me somewhere - its not like its going to cost you anything 
:-). If you need support you can try mailing me at the address above - I 
can be quite busy but I will try and deal with any queries.


GNU/Linux

Run: "make -f makefile.linux" and all of the objects will be compiled individually.


MacOS X

Run: "make -f makefile.darwin" and all of the objects will be compiled individually.


Windows 

There is a VC++ 6 project file included an it builds fine with that. I haven't 
tried anyother tools as yet. However there is a binary version included 
in case you haven't got the compiler.

To install - add mjLib.dll to your pd library path: 

	-lib C:\pd\mjLib\mjLib
	
and copy the contents of doc\mjLib  into 

	[pd home]\docs\5.reference\mjLib 
	
that should be you done.


General notes

This library will grow a bit - there are a few more objects that I want to 
put into it. 

There are currently five objects: 

	pin~	- randomly delivers the input signal to either the right or left outlet with a given probability
	metroplus - allows complex timing bangs to be delivered
	prob - generates random events with a given probability
	monorhythm - basic rhythm pattern building blocks that allows polyrhthms to be generated quickly and easily	
	about - delivers a number that is "about" the same as the input number.
	
	
mark williamson 
January 2002

___________________________________________________________

history: 

6th April 2004

<hans@at.or.at> added code to the Pd CVS and made all of the objects compile on
MacOS X and GNU/Linux.

1st February release 2

added new mode to monorhythm (exclusive - allows the beat and accent bangs to be mutually exclusive)
added about object

1st february release 1

added linux build files - not properly tested

31st January 2002

added prob and monorythm

30th January 2002

mods to metroplus to allow it to work just like metro is complex time mode not needed

29th january 2002

first release containing pin~ and metroplus
