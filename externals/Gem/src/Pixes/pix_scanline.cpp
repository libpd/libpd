/*
 *  pix_scanline.cpp
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_scanline.h"
CPPEXTERN_NEW(pix_scanline);

/////////////////////////////////////////////////////////
//
// pix_scanline
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_scanline :: pix_scanline()
{	
    
inletScanline = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("interlace"));

m_interlace = 0;
m_mode = 0;
 saved=NULL;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_scanline :: ~pix_scanline()
{
if(saved)delete saved;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_scanline :: processImage(imageStruct &image)
{
  int h,w,i,length,width,cleanup;
  long srcline,dstline;
  long interlace;

  interlace = m_interlace;
  if (interlace <= 0)interlace = 1;
  length = image.ysize /interlace; 
  width = image.xsize * image.csize;
  cleanup = image.ysize % interlace;
  srcline = 0;
  dstline = 0;
  if (m_mode == 0){
    for (h =0; h<length;h++){
      for (i = 0; i < interlace - 1; i++){
	dstline += width;
	for (w = 0; w < width;w++){
	  image.data[dstline+w] = image.data[srcline+w];
	}
      }
      srcline+= width * interlace;
      dstline += width;
    }
    if (cleanup) {
      for (i = 0; i < cleanup - 1; i++){
	dstline += width;
	for (w = 0; w < width;w++){
	  image.data[dstline+w] = image.data[srcline+w];
	}
      }
    }
  }else{
    for (h =0; h<length;h++){
      for (i = 0; i < interlace - 1; i++){
	dstline += width;
	for (w = 0; w < width;w++) image.data[dstline+w] = 0;
      }
      srcline+= width * interlace;
      dstline += width;
    }
    if (cleanup) {
      for (i = 0; i < cleanup - 1; i++){
	dstline += width;
	for (w = 0; w < width;w++)image.data[dstline+w] = 0;
      }
    }
  }
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_scanline :: processYUVImage(imageStruct &image)
{

    int h,w,i,length,width,cleanup;
    long srcline,dstline;
    long interlace;

interlace = m_interlace;
if (interlace <= 0){interlace = 1;}
length = image.ysize /interlace; 
width = image.xsize * image.csize;
cleanup = image.ysize % interlace;
//post("interlace %d cleanup %d ", interlace, cleanup);
srcline = 0;
dstline = 0;
if (m_mode == 0){
    for (h =0; h<length;h++){
        for (i = 0; i < interlace - 1; i++){
            dstline += width;
            for (w = 0; w < width;w++){
                image.data[dstline+w] = image.data[srcline+w];
                }
            }
            srcline+= width * interlace;
            dstline += width;
        }
        if (cleanup) {
        for (i = 0; i < cleanup - 1; i++){
            dstline += width;
            for (w = 0; w < width;w++){
                image.data[dstline+w] = image.data[srcline+w];
                }
            }
        
        }
    }else{
    for (h =0; h<length;h++){
        for (i = 0; i < interlace - 1; i++){
            dstline += width;
            for (w = 0; w < width;w+=2){
                image.data[dstline+w] = 128;
                image.data[dstline+w+1] = 0;
                }
            }
            srcline+= width * interlace;
            dstline += width;
        }
        if (cleanup) {
        for (i = 0; i < cleanup - 1; i++){
            dstline += width;
            for (w = 0; w < width;w+=2){
                image.data[dstline+w] = 128;
                image.data[dstline+w+1] = 0;
                }
            }
        
        }

    }
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_scanline :: obj_setupCallback(t_class *classPtr)
{

    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_scanline::rollCallback),
		  gensym("interlace"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_scanline::modeCallback),
		  gensym("mode"), A_DEFFLOAT, A_NULL);
}

void pix_scanline :: rollCallback(void *data, t_floatarg value)
{
  GetMyClass(data)->m_interlace=((long)value);
  GetMyClass(data)->setPixModified();
}

void pix_scanline :: modeCallback(void *data, t_floatarg value)
{
  GetMyClass(data)->m_mode=((long)value);
  GetMyClass(data)->setPixModified();
}

