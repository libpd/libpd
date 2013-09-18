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

#include "pix_resize.h"

#include <math.h>

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_resize, t_float,A_DEFFLOAT,t_float, A_DEFFLOAT);

/////////////////////////////////////////////////////////
//
// pix_resize
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_resize :: pix_resize(t_floatarg width=0, t_floatarg height=0)
{
  dimenMess((int)width, (int)height);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_resize :: ~pix_resize()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_resize :: processImage(imageStruct &image)
{
    // do we need to resize the image?
    // need to check if dimensions are a power of two 

    int wN = (m_width>0)?m_width:powerOfTwo(image.xsize);
    int hN = (m_height>0)?m_height:powerOfTwo(image.ysize);

    if (wN != image.xsize || hN != image.ysize)
    {
      GLint gluError;
      m_image.xsize=wN;
      m_image.ysize=hN;
      m_image.setCsizeByFormat(image.format);
      m_image.reallocate();
      m_image.reallocate(wN*hN*4); // just for safety: it seems like gluScaleImage needs more memory then just the x*y*c
      
      gluError = gluScaleImage(image.format,
			       image.xsize, image.ysize,
			       image.type, image.data,
			       wN, hN,
			       image.type, m_image.data);
      if ( gluError )
	{
	  post("gluError %d: unable to resize image", gluError);
	  return;
	}
      //      image.clear();
      image.data  = m_image.data;
      image.xsize = m_image.xsize;
      image.ysize = m_image.ysize;
    }
}

void pix_resize :: dimenMess(int width, int height) {
  if (width>32000)width=0;
  if (height>32000)height=0;
  if (width  < 0) width  = 0;
  if (height < 0) height = 0;

  m_width =width;
  m_height=height;

  setPixModified();
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_resize :: obj_setupCallback(t_class *classPtr)
{ 
  class_addmethod(classPtr, reinterpret_cast<t_method>(pix_resize::dimenMessCallback), 
		  gensym("dimen"), A_DEFFLOAT,A_DEFFLOAT, A_NULL);
}

void pix_resize ::dimenMessCallback(void *data, t_float w, t_float h)
{
  GetMyClass(data)->dimenMess((int)w, (int)h);
}
