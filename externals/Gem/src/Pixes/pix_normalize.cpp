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

#include "pix_normalize.h"

CPPEXTERN_NEW(pix_normalize);

/////////////////////////////////////////////////////////
//
// pix_normalize
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_normalize :: pix_normalize()
{ }

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_normalize :: ~pix_normalize()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_normalize :: processRGBAImage(imageStruct &image)
{
  unsigned char min=255, max=0;
  int datasize = image.xsize * image.ysize;// *image.csize;
  unsigned char *pixels = image.data;
  int n = datasize;

  while(n--){
    // think about this more carefully, to allow normalization for single channels...
    unsigned char red=pixels[chRed], green=pixels[chGreen], blue=pixels[chBlue];
    if (min>red)  min=red;
    if (min>green)min=green;
    if (min>blue) min=blue;
    if (max<red)  max=red;
    if (max<green)max=green;
    if (max<blue) max=blue;    
    pixels+=4;
  }

  t_float scale=(max-min)?255./(max-min):0;
  int iscale=static_cast<int>(scale*256);
 
  n = datasize*image.csize;
  pixels=image.data;
  while(n--){
    *pixels = static_cast<unsigned char>((*pixels-min)*iscale>>8);
    pixels++;
  }
}
void pix_normalize :: processGrayImage(imageStruct &image)
{
  unsigned char min=255, max=0;
  int datasize = image.xsize * image.ysize;
  unsigned char *pixels = image.data;
  int n = datasize;
  while(n--){
    int val=*pixels++;
    if (val>max)max=val;
    if (val<min)min=val;
  }
  pixels=image.data;
  n = datasize;
  if (max==min){
    memset(pixels, 0, datasize*sizeof(unsigned char));
  } else {
    t_float scale=(max-min)?255./(max-min):0;
    int iscale=static_cast<int>(scale*256);
    //    post("max=%d min=%d\t%f", max, min, scale);
    while(n--){
      int val=*pixels;
      //      if (n<2)post("n=%d\t%d %f %d", n, val, ((val-min)*scale), static_cast<unsigned char>((val-min)*scale));
      *pixels++= static_cast<unsigned char>((val-min)*iscale>>8);
    }
  }
}
void pix_normalize :: processYUVImage(imageStruct &image)
{
  unsigned char min=255, max=0;
  int datasize = image.xsize * image.ysize;// *image.csize;
  unsigned char *pixels = image.data;
  int n = datasize / 2;

  while(n--){
    // think about this more carefully, to allow normalization for single channels...
    unsigned char y0=pixels[chY0], y1=pixels[chY1];
    if (min>y0)  min=y0;
    if (min>y1)  min=y1;
    if (max<y0)  max=y0;
    if (max<y1)  max=y1;
    pixels+=4;
  }

  t_float scale=(max-min)?255./(max-min):0;
  int iscale=static_cast<int>(scale*256);

 
  n = datasize/2;
  pixels=image.data;
  while(n--){
    pixels[chY0] = static_cast<unsigned char>((pixels[chY0]-min)*iscale>>8);
    pixels[chY1] = static_cast<unsigned char>((pixels[chY1]-min)*iscale>>8);
    pixels+=4;
  }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_normalize :: obj_setupCallback(t_class *classPtr)
{}

