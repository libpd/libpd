////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "pix_delay.h"
#include <string.h>

CPPEXTERN_NEW_WITH_ONE_ARG(pix_delay, t_float,A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_delay
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_delay :: pix_delay(t_float &f)
{
  m_maxframes=(f>0)?(int)f:DEFAULT_MAX_FRAMES;
  myImage.xsize=myImage.ysize=myImage.csize=1;
  myImage.allocate(1*m_maxframes);
  m_curframe = m_frame = 0;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("delay"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_delay :: ~pix_delay()
{
  myImage.clear();
}

/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_delay :: delayMess(int frame)
{
  if (frame>=0)m_frame=(frame<m_maxframes)?frame:m_maxframes;
}


/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_delay :: processImage(imageStruct &image)
{
  unsigned char *src = image.data;
  unsigned char *dest;

  unsigned int dataSize = image.xsize * image.ysize * image.csize;
  int readframe;

  if (myImage.xsize*myImage.ysize*myImage.csize != image.xsize*image.ysize*image.csize){
    myImage.reallocate(dataSize*m_maxframes);
    m_curframe=0;
  }

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.setCsizeByFormat(image.format);
  myImage.reallocate();

  dest = myImage.data+m_curframe*dataSize;
  readframe=m_curframe-m_frame;
  readframe+=m_maxframes;
  readframe%=m_maxframes;

  // copy the data to the buffer
  //while(dataSize--)*src++=*dest++;
  memcpy(dest, src, dataSize);
  m_curframe++;
  m_curframe%=m_maxframes;  

  image.data=myImage.data+readframe*dataSize;
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_delay :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_delay::delayMessCallback),
  		  gensym("delay"), A_FLOAT, A_NULL);
}


void pix_delay :: delayMessCallback(void *data, t_floatarg frames)
{
  GetMyClass(data)->delayMess((int)frames);  
}
