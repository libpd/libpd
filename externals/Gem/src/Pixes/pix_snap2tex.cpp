///////////////////////////////////////////////////////
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
//    Copyright (c) 2003 Daniel Heckenberg
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_snap2tex.h"

#include "Gem/Manager.h"
#include "Utils/Functions.h"

CPPEXTERN_NEW_WITH_GIMME(pix_snap2tex);

/////////////////////////////////////////////////////////
//
// pix_snap2tex
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////

pix_snap2tex :: pix_snap2tex(int argc, t_atom *argv)
  : m_textureOnOff(1), m_textureQuality(GL_LINEAR),
    m_textureType(GL_TEXTURE_2D), m_repeat(GL_REPEAT),
    m_texWidth(-1), m_texHeight(-1),
    m_oldWidth(-1), m_oldHeight(-1),
    m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0),
    m_textureObj(0),
    m_didTexture(false)
{
  if (argc == 4)
    {
      m_x = atom_getint(&argv[0]);
      m_y = atom_getint(&argv[1]);
      m_width = atom_getint(&argv[2]);
      m_height = atom_getint(&argv[3]);
    }
  else if (argc == 2)
    {
      m_x = m_y = 0;
      m_width = atom_getint(&argv[0]);
      m_height = atom_getint(&argv[1]);
    }
  else if (argc == 0)
    {
      m_x = m_y = 0;
      m_width = m_height = -1;
    }
  else
    {
      error("needs 0, 2, or 4 values");
      m_x = m_y = 0;
      m_width = m_height = 128;
    }
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_pos"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("vert_size"));
  
  // create an outlet to send texture info
  m_outTexInfo = outlet_new(this->x_obj, 0);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_snap2tex :: ~pix_snap2tex()
{
}

/////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: setUpTextureState()
{
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);
  glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
  glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);

  if (m_mode && m_textureType !=  GL_TEXTURE_RECTANGLE_EXT)
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
}


/////////////////////////////////////////////////////////
// extension checks
//
/////////////////////////////////////////////////////////
bool pix_snap2tex :: isRunnable(void) {
  if(GLEW_VERSION_1_1 || GLEW_EXT_texture_object)
    return true;

  error("your system lacks texture support");
  return false;
}


/////////////////////////////////////////////////////////
// snapMess
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: snapMess()
{
  if(!GLEW_VERSION_1_1 && !GLEW_EXT_texture_object) return;
  
  int width  = m_width;
  int height = m_height;

  GemMan::getDimen(((m_width >0)?NULL:&width ),
		   ((m_height>0)?NULL:&height));

  if (width <= 0 || height <= 0)
    {
      error("Illegal size");
      return;
    }

  glEnable(m_textureType);

  if(GLEW_VERSION_1_1) {
    glBindTexture(m_textureType, m_textureObj);
  } else {
    glBindTextureEXT(m_textureType, m_textureObj);
  }

  // if the size changed, then reset the texture
  int x_2 = powerOfTwo(width);
  int y_2 = powerOfTwo(height);

  if (width != m_oldWidth || height != m_oldHeight) 
    {
      m_oldWidth = width;
      m_oldHeight = height;
      
      float m_xRatio = (float)width / (float)x_2;
      float m_yRatio = (float)height / (float)y_2;
		
      m_coords[0].s = 0.f;
      m_coords[0].t = 0.f;
		
      m_coords[1].s = m_xRatio;
      m_coords[1].t = 0.f;
		
      m_coords[2].s = m_xRatio;
      m_coords[2].t = m_yRatio;
		
      m_coords[3].s = 0.f;
      m_coords[3].t = m_yRatio;

      m_texWidth = x_2;
      m_texHeight = y_2;
		
      glCopyTexImage2D(	m_textureType, 0,
			GL_RGBA16,
			m_x, m_y,
			m_texWidth, m_texHeight, 
			0);
      
    } else {
    m_texHeight = m_height;
    m_texWidth = m_width;
  }
  
  glCopyTexSubImage2D(m_textureType, 0,
                      0, 0,
                      m_x, m_y,		// position
                      m_texWidth,
                      m_texHeight);		
  
  glDisable(m_textureType);

}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: render(GemState *state)
{
  m_didTexture=false;
  state->get(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->get(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->get(GemState::_GL_TEX_TYPE, m_oldTexture);

  if (!m_textureOnOff) return;

  state->set(GemState::_GL_TEX_COORDS, static_cast<TexCoord*>(m_coords));
  state->set(GemState::_GL_TEX_NUMCOORDS, 4);
  state->set(GemState::_GL_TEX_TYPE, 1);

  glEnable(m_textureType);

  if(GLEW_VERSION_1_1) {
    glBindTexture(m_textureType, m_textureObj);
  } else {
    glBindTextureEXT(m_textureType, m_textureObj);
  }

  m_didTexture=true;

  t_atom ap[4];
  SETFLOAT(ap, (t_float)m_textureObj);
  SETFLOAT(ap+1, m_texWidth);
  SETFLOAT(ap+2, m_texHeight);
  SETFLOAT(ap+3, m_textureType);
  // send texture info to outlet
  outlet_list(m_outTexInfo, 0, 4, ap);
}


/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: postrender(GemState *state)
{
  state->set(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->set(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->set(GemState::_GL_TEX_TYPE, m_oldTexture);

  if (m_didTexture){
    glDisable(m_textureType);
  }
}

/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: startRendering()
{
  if(GLEW_VERSION_1_1) {
    glGenTextures(1, &m_textureObj);
    glBindTexture(m_textureType, m_textureObj);
    setUpTextureState();
  } else {
    glGenTexturesEXT(1, &m_textureObj);
    glBindTextureEXT(m_textureType, m_textureObj);    
    setUpTextureState();
  }

  m_oldWidth = m_oldHeight = m_texWidth = m_texHeight = -1;
  if (!m_textureObj)	{
    error("Unable to allocate texture object");
    return;
  }

}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: stopRendering()
{
  if(m_textureObj) {
    if(GLEW_VERSION_1_1) {
      glDeleteTextures(1, &m_textureObj);
    } else {
      glDeleteTexturesEXT(1, &m_textureObj);
    }
  }
  m_textureObj = 0;
  m_oldWidth = m_oldHeight = m_texWidth = m_texHeight = -1;
}


/////////////////////////////////////////////////////////
// sizeMess
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: sizeMess(int width, int height)
{
  m_width = width;
  m_height = height;
  setModified();
}

/////////////////////////////////////////////////////////
// posMess
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: posMess(int x, int y)
{
  m_x = x;
  m_y = y;
  setModified();
}

/////////////////////////////////////////////////////////
// cleanImage
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: cleanImage()
{
  // release previous data
  error("clean is unimplemented.");
  setModified();
}

/////////////////////////////////////////////////////////
// textureOnOff
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: textureOnOff(int on)
{
  m_textureOnOff = on;
  setModified();
}

/////////////////////////////////////////////////////////
// textureQuality
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: textureQuality(int type)
{
  if (type)
    m_textureQuality = GL_LINEAR;
  else
    m_textureQuality = GL_NEAREST;
  if (m_textureObj)
    {
      if(GLEW_VERSION_1_1) {
        glBindTexture(m_textureType, m_textureObj);
        glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
        glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
      } else if (GLEW_EXT_texture_object) {
        glBindTextureEXT(m_textureType, m_textureObj);
        glTexParameteri(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
        glTexParameteri(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
      }
    }
  setModified();
}

/////////////////////////////////////////////////////////
// repeatMess
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: repeatMess(int type)
{
  if (type)
    m_repeat = GL_REPEAT;
  else
    m_repeat = GL_CLAMP_TO_EDGE;

  if (m_textureObj) {
    if(GLEW_VERSION_1_1) {
      glBindTexture(m_textureType, m_textureObj);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);
    } else if (GLEW_EXT_texture_object) {
      glBindTextureEXT(m_textureType, m_textureObj);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_S, m_repeat);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_T, m_repeat);
    }
  }
  setModified();
}

/////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_snap2tex :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::snapMessCallback),
		  gensym("snap"), A_NULL);
  class_addbang(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::snapMessCallback));

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::sizeMessCallback),
		  gensym("vert_size"), A_FLOAT, A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::posMessCallback),
		  gensym("vert_pos"), A_FLOAT, A_FLOAT, A_NULL);

  class_addfloat(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::floatMessCallback));    
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::textureMessCallback),
		  gensym("quality"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::repeatMessCallback),
		  gensym("repeat"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_snap2tex::modeCallback),
		  gensym("mode"), A_FLOAT, A_NULL);
}
void pix_snap2tex :: snapMessCallback(void *data)
{
  GetMyClass(data)->snapMess();
}
void pix_snap2tex :: sizeMessCallback(void *data, t_floatarg width, t_floatarg height)
{
  GetMyClass(data)->sizeMess(static_cast<int>(width), static_cast<int>(height));
}
void pix_snap2tex :: posMessCallback(void *data, t_floatarg x, t_floatarg y)
{
  GetMyClass(data)->posMess(static_cast<int>(x), static_cast<int>(y));
}
void pix_snap2tex :: floatMessCallback(void *data, float n)
{
  GetMyClass(data)->textureOnOff(static_cast<int>(n));
}
void pix_snap2tex :: textureMessCallback(void *data, t_floatarg quality)
{
  GetMyClass(data)->textureQuality(static_cast<int>(quality));
}
void pix_snap2tex :: repeatMessCallback(void *data, t_floatarg quality)
{
  GetMyClass(data)->repeatMess(static_cast<int>(quality));
}
void pix_snap2tex :: modeCallback(void *data, t_floatarg quality)
{
  GetMyClass(data)->m_mode=(static_cast<int>(quality));
}
