////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-2000 Mark Danks.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
// this is based on EffecTV by Fukuchi Kentarou
// * Copyright (C) 2001 FUKUCHI Kentarou
//
/////////////////////////////////////////////////////////

#include "pix_convert.h"

CPPEXTERN_NEW(pix_convert);

/////////////////////////////////////////////////////////
//
// pix_convert
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_convert :: pix_convert()
{
  m_image.xsize=128;
  m_image.ysize=128;
  m_image.csize=4;
  m_image.format=GL_RGBA;
  m_image.reallocate();
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_convert :: ~pix_convert()
{
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_convert :: processImage(imageStruct &image)
{
  if (image.format==m_image.format)return;
  m_image.xsize=image.xsize;
  m_image.ysize=image.ysize;

  switch(image.format){
  case GL_RGB:       m_image.fromRGB      (image.data); break;
  case GL_BGR:       m_image.fromBGR      (image.data); break;
  case GL_BGRA:      m_image.fromBGRA     (image.data); break;
  case GL_RGBA:      m_image.fromRGBA     (image.data); break;
  case GL_YUV422_GEM:m_image.fromYUV422   (image.data); break;
  case GL_LUMINANCE: m_image.fromGray     (image.data); break;
  default:
    post("no method for this format !!!");
    post("if you know how to convert this format (0x%X) to (0x%X),\n"
	 "please contact the authors of this software", image.format, m_image.format);
    return;
  }
  image.data  =m_image.data;
  image.csize =m_image.csize;
  image.format=m_image.format;

}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_convert :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_convert::colorMessCallback),
		  gensym("color"), A_SYMBOL, A_NULL);
}
void pix_convert :: colorMessCallback(void *data, t_symbol*s)
{
  int fo = getPixFormat(s->s_name);
  if(fo)GetMyClass(data)->m_image.setCsizeByFormat(fo);
  GetMyClass(data)->setPixModified();
}
