*****************************************************************************
copyleft 2003-2005 by Yves Degoyon ( ydegoyon@free.fr )

tarballs and updates available @ http://ydegoyon.free.fr
osx versions available @ http://homepage.mac.com/tigital

gemp2pdp : GEM to PDP bridge

To install gem2pdp, follow the steps from INSTALL

This software is published under GPL terms.

This is software with ABSOLUTELY NO WARRANTY.
Use it at your OWN RISK. It's possible to damage e.g. hardware or your hearing
due to a bug or for other reasons. 
We do not warrant that the program is free of infringement of any third-party
patents.

*****************************************************************************


notes:

- now allows you to select the colorspace that pdp's yv12 image format is converted to.  This means we can now request a YUV colorspace, which is computationally much faster than yv12 to RGB or RGBA (so much so that it is the default).  Colorspace can be indicated on object creation (ie. [pdp2gem RGB]) or by message (ie. [colorspace YUV<)

-  now uses standardized color conversions found in GemPixUtil.cpp, which includes altivec speedups!  These use formulae from http://www.poynton.com/notes/colour_and_gamma/ColorFAQ.html#RTFToC30

- please note that colorspace switching between objects is handled automatically by GEM unless specifically stated (ie. by [pix_yuv], [pix_rgba], [pix_grey])