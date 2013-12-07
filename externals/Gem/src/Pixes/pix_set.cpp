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

/////////////////////////////////////////////////////////
//
//  pix_set
//
//  0409:forum::für::umläute:2000
//  IOhannes m zmoelnig
//  mailto:zmoelnig@iem.kug.ac.at
//
/////////////////////////////////////////////////////////



// this is to paint easily your own pictures by passing a package of floats that contain all necessary image data)

#include "pix_set.h"
#include "Gem/State.h"

#include <string.h>

CPPEXTERN_NEW_WITH_TWO_ARGS(pix_set, t_floatarg, A_DEFFLOAT, t_floatarg, A_DEFFLOAT);

/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_set :: pix_set(t_floatarg xsize, t_floatarg ysize)
{
  int dataSize;
  if (xsize < 1) xsize = 256;
  if (ysize < 1) ysize = 256;

  m_pixBlock.image = m_imageStruct;
  m_pixBlock.image.xsize = (int)xsize;
  m_pixBlock.image.ysize = (int)ysize;
  m_pixBlock.image.csize = 4;
  m_pixBlock.image.format = GL_RGBA;
  m_pixBlock.image.type = GL_UNSIGNED_BYTE;
  
  dataSize = m_pixBlock.image.xsize * m_pixBlock.image.ysize * 
    m_pixBlock.image.csize * sizeof(unsigned char);
  m_pixBlock.image.allocate(dataSize);
  
  memset(m_pixBlock.image.data, 0, dataSize);
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("data"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_set :: ~pix_set()
{
  cleanPixBlock();
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_set :: render(GemState *state)
{
  state->set(GemState::_PIX,&m_pixBlock);
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_set :: startRendering()
{
    m_pixBlock.newimage = 1;
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_set :: postrender(GemState *state)
{
    m_pixBlock.newimage = 0;
    //state->image = NULL;
}


/////////////////////////////////////////////////////////
// DATAMess
//
/////////////////////////////////////////////////////////
void pix_set :: DATAMess(int argc, t_atom *argv)
{
  int picturesize = m_pixBlock.image.xsize * m_pixBlock.image.ysize, counter, n;
  unsigned char *buffer = m_pixBlock.image.data;
  
  //	argc--;
  memset(buffer, 0, picturesize*m_pixBlock.image.csize*sizeof(unsigned char));
	
  switch (m_mode) {
  case GL_RGB:
    n = argc/3;
    counter=(picturesize<n)?picturesize:n;
    while (counter--) {
      buffer[0] = (unsigned char)(255.*atom_getfloat(&argv[0])); // red
      buffer[1] = (unsigned char)(255.*atom_getfloat(&argv[1])); // green
      buffer[2] = (unsigned char)(255.*atom_getfloat(&argv[2])); // blue
      buffer[3] = 0;					     // alpha
      argv+=3; buffer+=4;
    }
    break;
  case GL_LUMINANCE:
    counter=(picturesize<argc)?picturesize:argc;
    while (counter--) {
      buffer[0] = buffer[1] = buffer[2] = (unsigned char)(255.*atom_getfloat(argv));	// rgb
      buffer[3] = 0;									// alpha
      argv++;	buffer+=4;
    }
    break;
  case GL_YCBCR_422_GEM:
    // ?
    break;
  default:
    n = argc/4;
    counter=(picturesize<n)?picturesize:n;
    while (counter--) {
      buffer[0] = (unsigned char)(255.*atom_getfloat(&argv[0])); // red
      buffer[1] = (unsigned char)(255.*atom_getfloat(&argv[1])); // green
      buffer[2] = (unsigned char)(255.*atom_getfloat(&argv[2])); // blue
      buffer[3] = (unsigned char)(255.*atom_getfloat(&argv[3])); // alpha
      argv+=4; buffer+=4;
    }
  }
  m_pixBlock.newimage = 1;
}



/////////////////////////////////////////////////////////
// RGBAMess
//
/////////////////////////////////////////////////////////
void pix_set :: RGBAMess(void)
{
	m_mode = GL_RGBA;
}
/////////////////////////////////////////////////////////
// RGBMess
//
/////////////////////////////////////////////////////////
void pix_set :: RGBMess(void)
{
	m_mode = GL_RGB;
}
/////////////////////////////////////////////////////////
// GREYMess
//
/////////////////////////////////////////////////////////
void pix_set :: GREYMess(void)
{
	m_mode = GL_LUMINANCE;
}

/////////////////////////////////////////////////////////
// SETMess
//
/////////////////////////////////////////////////////////
void pix_set :: SETMess(int xsize, int ysize)
{
	int dataSize;
	if ((xsize < 1) || (ysize < 1)) return;
	m_pixBlock.image.clear();
	m_pixBlock.image.xsize = (int)xsize;
	m_pixBlock.image.ysize = (int)ysize;
	m_pixBlock.image.csize = 4;
	m_pixBlock.image.format = GL_RGBA;
	m_pixBlock.image.type = GL_UNSIGNED_BYTE;
	
	dataSize = m_pixBlock.image.xsize * m_pixBlock.image.ysize
		* 4 * sizeof(unsigned char);
	m_pixBlock.image.allocate(dataSize);
	memset(m_pixBlock.image.data, 0, dataSize);
}

/////////////////////////////////////////////////////////
// cleanPixBlock -- free the pixel buffer memory
//
/////////////////////////////////////////////////////////
void pix_set :: cleanPixBlock()
{
  m_pixBlock.image.clear();
  m_pixBlock.image.data = NULL;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_set :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::RGBAMessCallback),
		gensym("RGBA"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::RGBMessCallback),
		gensym("RGB"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::GREYMessCallback),
		gensym("GREY"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::GREYMessCallback),
		gensym("GRAY"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::YUVMessCallback),
		gensym("YUV"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::RGBAMessCallback),
		gensym("rgba"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::RGBMessCallback),
		gensym("rgb"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::GREYMessCallback),
		gensym("grey"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::GREYMessCallback),
		gensym("gray"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::YUVMessCallback),
		gensym("yuv"), A_NULL);
     class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::SETMessCallback),
		gensym("set"), A_FLOAT, A_FLOAT, A_NULL);
	
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_set::DATAMessCallback),
		gensym("data"), A_GIMME, A_NULL);
}

void pix_set :: RGBAMessCallback(void *data)
{
	GetMyClass(data)->m_mode=GL_RGBA;
}

void pix_set :: RGBMessCallback(void *data)
{
	GetMyClass(data)->m_mode=GL_RGB;
}

void pix_set :: GREYMessCallback(void *data)
{
	GetMyClass(data)->m_mode=GL_LUMINANCE;
}
void pix_set :: YUVMessCallback(void *data)
{
  //	GetMyClass(data)->m_mode=GL_YCBCR_422_GEM;
}
void pix_set :: SETMessCallback(void *data, t_float x, t_float y)
{
    GetMyClass(data)->SETMess((int)x, (int)y);
}

void pix_set :: DATAMessCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
    GetMyClass(data)->DATAMess(argc, argv);
}
