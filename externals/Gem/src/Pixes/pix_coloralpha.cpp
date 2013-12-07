////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1998 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_coloralpha.h"

CPPEXTERN_NEW(pix_coloralpha);

/////////////////////////////////////////////////////////
//
// pix_coloralpha
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_coloralpha :: pix_coloralpha()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_coloralpha :: ~pix_coloralpha()
{ }

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_coloralpha :: processRGBAImage(imageStruct &image)
{
    // process the image
    int datasize = image.xsize * image.ysize;
    unsigned char *pixel = image.data;

    while(datasize--)
    {		
                /*
		float tempVal = (float)pixel[chRed] + (float)pixel[chGreen] + (float)pixel[chBlue];
		tempVal /= 3.f;
		pixel[chAlpha] = (unsigned char)tempVal;
		pixel += 4;
                */
                int tempVal = pixel[chRed] + pixel[chGreen] + pixel[chBlue];
		tempVal /= 3;
		pixel[chAlpha] = (unsigned char)tempVal;
		pixel += 4;
    }    
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_coloralpha :: obj_setupCallback(t_class *)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_coloralpha), 
		   gensym("pix_colouralpha"), A_NULL);
}
