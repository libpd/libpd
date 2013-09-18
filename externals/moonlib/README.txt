Here are the libs I've written for Pd.
They can be splitted in three parts:

   1. the nilib library, which is a kind of wrapper between Pd and Gtk, and its
    objects:
 	nifs (a one-click file selector, with some special tricks...), niscope (a 
	simple oscilloscope), and nileon (a simple drum machine, but it's the 
	replication of an enormous mechanical one, named Leon Napakatbra, which was 
	built from a 8 meters diameter merry-go-round...). 
	 There was also nimouse (giving mouse position/buttons) and nitab (the same 
	for wacom serial graphire tablet) but i don't use these anymore: it's too 
	dangerous to access a device file directly from pd. Beter use gtkmouse and 
	wac/wacusb programs with pdsend/pdreceive.

   2. the sub library, which is a collection of gui control objects (subdial, 
   	subslider, subbang and subtoggle) that are placed into a subpatch but are 
	visible in the parent window, into the subpatch's box. The idea was to build 
	easy-to-use complex subpatchs, or abstractions. I wrote it before 
	"graph-on-parent" was implemented in Pd, so it's a bit deprecated now, but 
	it has the advantage that subs values (positions of the buttons) are saved 
	into the parent patch... very useful in case of abstractions.
     About of these questions, have a look to my AutoPreset abstractions, which 
	adds a manner to save the value of regular gui objects (hslider etc...) into 
	either a file or the parent patch, even those nested in abstractions (and 
	recursively in abstractions of abstractions...). It works also with tables, 
	symbols and symbol arrays.

   3. some other objects, such as:

    tabenv : like env~, an enveloppe follower, but computing on a table, so 
	 possibly much speeder than real-time env~'s computation.
    tabsort and tabsort2 : returns the indices of the sorted table (tabsort2 
	 is bidimentionnal).
    gamme : one octave of a piano keyboard used to filter/choose notes in a 
	 selected scale.
    absolutepath/relativepath : to use datas (sounds, texts, presets, images, 
	 programs...) nested in the patch's directory (and in subdirs).
    sarray and slist : to creates shared dynamic arrays or lists with symbols.
    sfread2~ and readsfv~ : to pitch the direct-from-disk reading of sound files.
    dinlet~ : an inlet~ with a default value (when nothing is connected to it).
    mknob : a round knob ala iemgui vslider (with its "properties" window).
    dispatch : creates one bus name for many buttons' buses: from the N pairs  
	 (slider1-snd/slider1-rcv) ... (sliderN-snd/sliderN-rcv), creates only 
	 one pair of buses named (slider-snd/slider-rcv) , in which datas are 
	 prepended by the number of the "sub-bus".
    joystik : an improvment of Joseph A. Sarlo's joystick.
    image :  an improvment  of Guenter Geiger's one. Same name, but it's 
	 compatible. Here you can share images through different objects, preload 
	 a list of images, and animate this list.

    and some others...

CAUTION:

    This stuff was written with Pd v0.34-4 to v0.37 on a PC under Linux. It 
	hasn't be tested under other conditions; nilib will only work under Linux 
	(but I think it will be OK on other machines than PC), because of use of 
	multithreading and GTK .
    Anyway makefiles are only working for Linux for the moment...
    Moreover I think sub library is potentially very sensible to possible 
	modifications of Pd sources.



To install:

sublib_0.1.0 is for Pd 0.33
sublib_0.2.0 is for Pd 0.35
sublib_0.3.0 is for Pd 0.37

nilib_0.1.0 is for Pd 0.35
nilib_0.2.0 is for Pd 0.37

others_0.1.0 is for Pd 0.35 
others_0.2.0 is for Pd 0.37



Edit paths.txt to tune install paths and pure-data's location.
Edit this makefile if you want to select older versions of externals (if you
 are using pd0.37 it should be ok).

Then:

make
make install



If you keep externals locally inside moonlibs directory, then you should add it
to your pdrc file:

-path moonlibs-0.2/externs
-helppath moonlibs-0.2/docs


CAUTION : 
	1)	(pd 0.35) You MUST fix a bug pd sources and recompile them in order to have 
	dinlet~ working !! 
	You have to replace the function obj_findsignalscalar() in file m_obj.c 
	by the one written in dinlet~.c .
	
	(dinlet~ is a signal inlet where you can choose the default float value it
	outputs when no signal is connected into.)


	2) In order to have sfread~ working with big files in direct-from-disk 
	mode you have to hack pd sources: change 
		mlockall(MCL_FUTURE) 
	with 
		mlockall(MCL_CURRENT)
	in s_linux.c (pd0.35) or s_inter.c (pd0.37). If not the whole file will be loaded in memory when
	opening it.
