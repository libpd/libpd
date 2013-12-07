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
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_mask.h"
#include "Gem/PixConvert.h"

CPPEXTERN_NEW(pix_mask);

/////////////////////////////////////////////////////////
//
// pix_mask
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_mask :: pix_mask()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_mask :: ~pix_mask()
{ }

/////////////////////////////////////////////////////////
// processDualImage
//
/////////////////////////////////////////////////////////
void pix_mask :: processRGBA_RGBA(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  unsigned char *mask = right.data;

  while (datasize--)	{
    // calculate the alpha value
    unsigned short grey=(mask[chRed  ] * RGB2GRAY_RED  + mask[chGreen] * RGB2GRAY_GREEN + mask[chBlue ] * RGB2GRAY_BLUE);
    pixels[chAlpha] = grey>>8;
    pixels += 4;
    mask += 4;
  }
}

/////////////////////////////////////////////////////////
// processRightGray
//
/////////////////////////////////////////////////////////
void pix_mask :: processRGBA_Gray(imageStruct &image, imageStruct &right)
{
  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  unsigned char *mask = right.data;

  while(datasize--)	{
    pixels[chAlpha] = *mask++;
    pixels += 4;
  }
}

/////////////////////////////////////////////////////////
// processRightGray
//
/////////////////////////////////////////////////////////
void pix_mask :: processRGBA_YUV(imageStruct &image, imageStruct &right)
{
  int datasize = (image.xsize * image.ysize)>>1;
  unsigned char *pixels = image.data;
  unsigned char *mask = right.data;

  while(datasize--)	{
    pixels[chAlpha] = mask[chY0];
    pixels += 4;
    pixels[chAlpha] = mask[chY1];
    pixels += 4;
    mask   += 4;


  }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_mask :: obj_setupCallback(t_class *)
{ }
