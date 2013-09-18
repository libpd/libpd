freeverb~ version 1.2
reverb external for Pure Data and Max/MSP
written by Olaf Matthes <olaf.matthes@gmx.de>

based on Freeverb, the free, studio-quality reverb SOURCE CODE in the public 
domain, Written by Jezar at Dreampoint - http://www.dreampoint.co.uk

This software is published under GPL terms, see file LICENSE.

This is software with ABSOLUTELY NO WARRANTY.
Use it at your OWN RISK. It's possible to damage e.g. hardware or your hearing
due to a bug or for other reasons. 

Recent changes:
- added check for NANs
- added a hand unrolled version of the perform routine for DSP vector sizes that
  are a multiple of 8. This should speed up things a bit


Below some notes taken from Freeverb readme:
-------------------------------------------------------------------------------------

Note that this version of Freeverb doesn't contain predelay, or any EQ. I thought 
that might make it difficult to understand the "reverb" part of the code. Once you 
figure out how Freeverb works, you should find it trivial to add such features with 
little CPU overhead.

Technical Explanation 
---------------------

Freeverb is a simple implementation of the standard Schroeder/Moorer reverb model. 
I guess the only reason why it sounds better than other reverbs, is simply because 
I spent a long while doing listening tests in order to create the values found in "tuning.h". It uses 8 comb filters on both the left and right channels), and you 
might possibly be able to get away with less if CPU power is a serious constraint 
for you. It then feeds the result of the reverb through 4 allpass filters on both 
the left and right channels. These "smooth" the sound. Adding more than four allpasses 
doesn't seem to add anything significant to the sound, and if you use less, the sound 
gets a bit "grainy". The filters on the right channel are slightly detuned compared 
to the left channel in order to create a stereo effect.

