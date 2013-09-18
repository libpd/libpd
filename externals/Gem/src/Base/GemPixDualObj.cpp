////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "GemPixDualObj.h"

#include "Gem/Cache.h"
#include "Gem/State.h"

#include <string.h>
#include <stdio.h>

/////////////////////////////////////////////////////////
//
// GemPixDualObj
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
GemPixDualObj :: GemPixDualObj()
  : m_cacheRight(NULL),
    m_pixRight(NULL),// was dsiabled by DH 8/5/02
    m_pixRightValid(-1),
    org_pixRightValid(-1),
    m_inlet(NULL)
{
    m_inlet = inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("gem_state"), gensym("gem_right"));
    memset(&m_pixRight, 0, sizeof(m_pixRight));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
GemPixDualObj :: ~GemPixDualObj()
{
    inlet_free(m_inlet);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void GemPixDualObj :: render(GemState *state)
{
  GemPixObj::render(state);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
#define PROCESS_DUALIMAGE_SIMD(CS) \
  switch(m_simd){		   \
  case (GEM_SIMD_MMX):				\
    process##CS ##_MMX(image, m_pixRight->image);	\
    break;					\
  case(GEM_SIMD_SSE2):				\
    process##CS ##_SSE2(image, m_pixRight->image);	\
    break;					\
  case(GEM_SIMD_ALTIVEC):				\
    process##CS ##_Altivec(image, m_pixRight->image);		\
    break;						\
  default:						\
    process##CS ##_##CS(image, m_pixRight->image);		\
  }
  
#define PROCESS_DUALIMAGE(CS1, CS2) \
  process##CS1 ##_##CS2 (image, m_pixRight->image);

#define PROCESS_COLORSPACE(FUN_RGBA, FUN_YUV, FUN_GRAY)			\
  switch (m_pixRight->image.format) {					\
  case GL_RGBA: case GL_BGRA_EXT:					\
    found=true; FUN_RGBA; break;		\
  case GL_LUMINANCE:							\
    found=true; FUN_GRAY; break;		\
  case GL_YCBCR_422_GEM:						\
    found=true; FUN_YUV ; break;		\
  default:break;}

void GemPixDualObj :: processImage(imageStruct &image)
{
  if (!m_cacheRight || m_cacheRight->m_magic!=GEMCACHE_MAGIC){
    m_cacheRight=NULL;
    return;
  }

  //if (!m_cacheRight || !&image || !&m_pixRight || !&m_pixRight->image) return;
  if (!m_pixRightValid || !&image || !&m_pixRight || !&m_pixRight->image) return;
  
    if (image.xsize != m_pixRight->image.xsize ||
    	image.ysize != m_pixRight->image.ysize)    {
      error("two images do not have equal dimensions (%dx%d != %dx%d)", 
	    image.xsize, image.ysize,
	    m_pixRight->image.xsize, m_pixRight->image.ysize);
      m_pixRightValid = 0;  
    	return;
    }

    if(image.upsidedown != m_pixRight->image.upsidedown) {
      image.fixUpDown();
      m_pixRight->image.fixUpDown();
    }

    bool found = false;
    switch (image.format) {
    case GL_RGBA:
    case GL_BGRA_EXT:
      PROCESS_COLORSPACE(PROCESS_DUALIMAGE_SIMD(RGBA),
			 PROCESS_DUALIMAGE(RGBA, YUV),
			 PROCESS_DUALIMAGE(RGBA, Gray));
      break;
    case GL_LUMINANCE:
      PROCESS_COLORSPACE(PROCESS_DUALIMAGE(Gray, RGBA),
			 PROCESS_DUALIMAGE(Gray, YUV),
			 PROCESS_DUALIMAGE_SIMD(Gray));
      break;
    case GL_YCBCR_422_GEM:
      PROCESS_COLORSPACE(PROCESS_DUALIMAGE(YUV, RGBA),
			 PROCESS_DUALIMAGE_SIMD(YUV),
			 PROCESS_DUALIMAGE(YUV, Gray));
      break;
    default: break;
    }
    if (!found)processDualImage(image, m_pixRight->image);
}

/////////////////////////////////////////////////////////
// process
//
/////////////////////////////////////////////////////////

void GemPixDualObj :: processDualImage(imageStruct &left, imageStruct &right){
  char *lformat, *rformat;
  switch (left.format) {
  case GL_RGBA:
  case GL_BGRA_EXT:
    lformat =(char*)"RGBA";break;
  case GL_LUMINANCE:
    lformat =(char*)"Gray";break;
  case GL_YCBCR_422_GEM:
    lformat =(char*)"YUV";break;
  default:
    lformat = new char[6];
    sprintf(lformat,"0x%04X", (unsigned int)left.format);
  }
  switch (right.format) {
  case GL_RGBA:
  case GL_BGRA_EXT:
    rformat =(char*)"RGBA";break;
  case GL_LUMINANCE:
    rformat =(char*)"Gray";break;
  case GL_YCBCR_422_GEM:
    rformat =(char*)"YUV";break;
  default:
    rformat = new char[6];
    sprintf(rformat, "0x%04X", (unsigned int)left.format);
  }
  
  error("no method to combine (%s) and (%s)", lformat, rformat);
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void GemPixDualObj :: postrender(GemState *state)
{
  if (org_pixRightValid != m_pixRightValid)setPixModified();

  org_pixRightValid = m_pixRightValid;

  m_pixRightValid = 0;
}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void GemPixDualObj :: stopRendering()
{
  m_pixRightValid = 0;
}

/////////////////////////////////////////////////////////
// rightRender
//
/////////////////////////////////////////////////////////
void GemPixDualObj :: rightRender(GemState *statePtr)
{
  if (!statePtr || !statePtr->get(GemState::_PIX, m_pixRight) || !m_pixRight) {
    m_pixRightValid = 0;
    m_pixRight = 0;
    return;
  }
  
  m_pixRightValid = 1;
  if (m_pixRight->newimage)setPixModified(); // force the left arm to create a new image
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void GemPixDualObj :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&GemPixDualObj::gem_rightMessCallback),
    	    gensym("gem_right"), A_GIMME, A_NULL);
}
void GemPixDualObj :: gem_rightMessCallback(void *data, t_symbol *s, int argc, t_atom *argv)
{
  if (argc==1 && argv->a_type==A_FLOAT){
  } else if (argc==2 && argv->a_type==A_POINTER && (argv+1)->a_type==A_POINTER){
    GetMyClass(data)->m_cacheRight = (GemCache*)argv->a_w.w_gpointer;
    GetMyClass(data)->rightRender((GemState *)(argv+1)->a_w.w_gpointer);
  } else GetMyClass(data)->error("wrong righthand arguments....");
}
