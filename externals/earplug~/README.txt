/* RT binaural filter: earplug~        */
/* based on KEMAR impulse measurement  */
/* Pei Xiang, summer 2004              */
/* Revised in Fall 2006 by Jorge Castellanos */
/* Revised in Spring 2009 by Hans-Christoph Steiner to compile in the data file */

http://markmail.org/message/sxfauaymftshbgwz
http://lists.puredata.info/pipermail/pd-list/2005-02/025764.html

actually i've just written one external that handles this , [earplug~] http://crca.ucsd.edu/~pxiang/research.htm it basically takes the KEMAR data set, and interpolates 366 locations where HRTF measurement exists in a spherical surface. you get azimuth control 0-360 and elevation -40 - 90.

now i'm still trying to clean it up a little bit, but probably have to use by copy the data.txt file into the default search dir of Pd, for the moment.

cheers, Pei 
