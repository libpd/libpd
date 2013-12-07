/*
 *  pix_roll.cpp
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#include "pix_roll.h"

#include <string.h>

CPPEXTERN_NEW(pix_roll);

/////////////////////////////////////////////////////////
//
// pix_roll
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_roll :: pix_roll()
{	long size,src;
    
  inletBlur = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("roll"));

  m_vroll = 0;
  m_blurH = 240;
  m_blurW = 240;
  m_blurBpp = 2;
  size = 320 * 240 * 4;
  saved = new unsigned char [size];
  src=0;
  m_axis = 0;

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_roll :: ~pix_roll()
{
  if(saved)delete saved;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_roll :: processRGBAImage(imageStruct &image)
{
  int h,w;
  long rollstart;

  unsigned char *srcdata;
  unsigned char *dstdata;

  if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {
    m_blurH    = image.ysize;
    m_blurW    = image.xsize;
    m_blurBpp  = image.csize;
    m_blurSize = m_blurH * m_blurW * m_blurBpp;
    if(saved)delete saved;
    saved = new unsigned char [m_blurSize];
  }    
  if (!m_axis) {
    m_vroll%=image.ysize;
    m_vroll+=image.ysize;
    m_vroll%=image.ysize;
    long linelength=image.xsize*image.csize;
    memcpy(saved, image.data+linelength*m_vroll, linelength*(image.ysize-m_vroll));
    memcpy(saved+linelength*(image.ysize-m_vroll), image.data, linelength*m_vroll);
  }else{
    m_vroll%=image.xsize;
    m_vroll+=image.xsize;
    m_vroll%=image.xsize;
    rollstart = m_vroll * image.csize;
    //moved the rolled part of the data
    srcdata=image.data+rollstart;
    dstdata=saved;
    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
      //      for(w=m_vroll; w<image.xsize; w++){//from rollstart to the last column
      w=((image.xsize-m_vroll)*image.csize)/4;
      while(w--){
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
      }
      srcdata+=rollstart;
      dstdata+=rollstart;
    }
    //finish up
    srcdata=image.data;
    dstdata=saved+(image.xsize-m_vroll)*image.csize;
    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
      //      for(w=image.xsize-m_vroll; w<image.xsize; w++){//from rollstart to the last column
      w=(m_vroll*image.csize)/4;
      while(w--){
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
        *dstdata++=*srcdata++;
      }
      srcdata+=(image.xsize*image.csize-rollstart);
      dstdata+=(image.xsize*image.csize-rollstart);
    }
  }
  //  memcpy(image.data,saved,pixsize);     
  image.data=saved;
}
void pix_roll :: processImage(imageStruct &image)
{
  int h,w;
  long rollstart;

  unsigned char *srcdata;
  unsigned char *dstdata;

  if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {
    m_blurH    = image.ysize;
    m_blurW    = image.xsize;
    m_blurBpp  = image.csize;
    m_blurSize = m_blurH * m_blurW * m_blurBpp;
    if(saved)delete saved;
    saved = new unsigned char [m_blurSize];
  }    
  if (!m_axis) {
    m_vroll%=image.ysize;
    m_vroll+=image.ysize;
    m_vroll%=image.ysize;
    long linelength=image.xsize*image.csize;
    memcpy(saved, image.data+linelength*m_vroll, linelength*(image.ysize-m_vroll));
    memcpy(saved+linelength*(image.ysize-m_vroll), image.data, linelength*m_vroll);
  }else{
    m_vroll%=image.xsize;
    m_vroll+=image.xsize;
    m_vroll%=image.xsize;
    rollstart = m_vroll * image.csize;
    //moved the rolled part of the data
    srcdata=image.data+rollstart;
    dstdata=saved;
    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
      //      for(w=m_vroll; w<image.xsize; w++){//from rollstart to the last column
      w=((image.xsize-m_vroll)*image.csize);
      while(w--)*dstdata++=*srcdata++;
      srcdata+=rollstart;
      dstdata+=rollstart;
    }
    //finish up
    srcdata=image.data;
    dstdata=saved+(image.xsize-m_vroll)*image.csize;
    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
      //      for(w=image.xsize-m_vroll; w<image.xsize; w++){//from rollstart to the last column
      w=(m_vroll*image.csize);
      while(w--)*dstdata++=*srcdata++;
      srcdata+=(image.xsize*image.csize-rollstart);
      dstdata+=(image.xsize*image.csize-rollstart);
    }
  }
  //  memcpy(image.data,saved,pixsize);     
  image.data=saved;
}
/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_roll :: processYUVImage(imageStruct &image)
{

  int h,w,length;
  long src,dst;
  long rollstart,pixsize;

  if (m_blurH != image.ysize || m_blurW != image.xsize || m_blurBpp != image.csize) {

    m_blurH = image.ysize;
    m_blurW = image.xsize;
    m_blurBpp = image.csize;
    m_blurSize = m_blurH * m_blurW * m_blurBpp;
    if(saved)delete saved;
    saved = new unsigned char [m_blurSize];
  }    
  pixsize = image.ysize * image.xsize * image.csize;
  if (!m_axis) {
    //start at the breakpoint
    dst = 0;
    m_vroll%=image.ysize;
    m_vroll+=image.ysize;
    m_vroll%=image.ysize;
    rollstart = m_vroll * image.xsize * image.csize;

    for (src = rollstart; src < pixsize;src++){
      saved[dst] = image.data[src];
      dst++;
    }
    //finish the rest
    for(src = 0;src<rollstart-1;src++){
      saved[dst] = image.data[src];
      dst++;
    }
  }else{
    if (m_vroll % 2 ){m_vroll += 1;}//check for odd in YUV
    m_vroll%=image.ysize;
    m_vroll+=image.ysize;
    m_vroll%=image.ysize;

    rollstart = m_vroll * image.csize;
    dst = 0;
    src=rollstart;
    length = (image.xsize-m_vroll)*2;
    //moved the rolled part of the data
    src = rollstart;//offset source to start of roll
    dst = 0; 

    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
   
      for(w=m_vroll; w<image.xsize; w++){//from rollstart to the last column
        
        saved[dst] = image.data[src];
        saved[dst+1] = image.data[src+1];
        
        src+=2;dst+=2;
      }
      src+= rollstart;
      dst+= rollstart;
    }
    //finish up
    src=0;
    dst=(image.xsize-m_vroll)*2;
    for (h=0; h<image.ysize; h++){ //fixed...we have to do every row
   
      for(w=image.xsize-m_vroll; w<image.xsize; w++){//from rollstart to the last column
        
        saved[dst] = image.data[src];
        saved[dst+1] = image.data[src+1];
        
       
        src+=2;dst+=2;
      }
      src+= image.xsize*2-rollstart;
      dst+= image.xsize*2-rollstart;
    }
  }
  memcpy(image.data,saved,pixsize);     

}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_roll :: obj_setupCallback(t_class *classPtr)
{

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_roll::rollCallback),
                  gensym("roll"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_roll::axisCallback),
                  gensym("axis"), A_DEFFLOAT, A_NULL);
}

void pix_roll :: rollCallback(void *data, t_floatarg value)
{
  GetMyClass(data)->m_vroll=((long)value);
  GetMyClass(data)->setPixModified();
}

void pix_roll :: axisCallback(void *data, t_floatarg value)
{
  GetMyClass(data)->m_axis=((long)value);
  GetMyClass(data)->setPixModified();
}

