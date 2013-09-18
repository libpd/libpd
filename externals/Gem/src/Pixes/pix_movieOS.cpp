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
//    Copyright (c) 2002 James Tittle
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////
#include "Gem/GemConfig.h"

#if defined GEM_FILMBACKEND && GEM_FILMBACKEND != GEM_FILMBACKEND_Darwin

#include "pix_movieOS.h"

#include "Gem/Manager.h"

CPPEXTERN_NEW_WITH_ONE_ARG(pix_movieOS, t_symbol *, A_DEFSYM);

/////////////////////////////////////////////////////////
//
// pix_movieOS
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_movieOS :: pix_movieOS(t_symbol *filename) :
#if __APPLE__
  pix_filmDarwin(filename)
#else
# error define pix_film for your OS
#endif
  , 
   m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0), 
   m_textureObj(0), m_xRatio(1.f), m_yRatio(1.f)
{
  m_film=false;
  post("I should never be called on a Mac");
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_movieOS :: ~pix_movieOS()
{
  // Clean up the movie
  closeMess();
  deleteBuffer();
}

/////////////////////////////////////////////////////////
// Buffer for Frames
//
/////////////////////////////////////////////////////////
void pix_movieOS :: createBuffer()
{
  int neededXSize = powerOfTwo(m_xsize);
  int neededYSize = powerOfTwo(m_ysize);
  
  deleteBuffer();
  int dataSize = neededXSize * neededYSize * m_csize;
  m_pixBlock.image.data = new unsigned char[dataSize];
  m_data=m_pixBlock.image.data; // ??????????????
  memset(m_pixBlock.image.data, 0, dataSize);
  m_frame =/*(char*)*/m_pixBlock.image.data;

  m_pixBlock.image.xsize  = neededXSize;
  m_pixBlock.image.ysize  = neededYSize;
  m_pixBlock.image.csize  = m_csize;
  m_pixBlock.image.format = m_format;
}

/////////////////////////////////////////////////////////
// on opening a file, prepare for texturing
//
/////////////////////////////////////////////////////////
void pix_movieOS :: prepareTexture()
{
    if (!GemMan::texture_rectangle_supported)
    {
        int neededXSize = m_pixBlock.image.xsize;
        int neededYSize = m_pixBlock.image.ysize;
        post("prepareTexture: x : %d, y : %d", neededXSize, neededYSize );

        // ratio for the texture map coordinates
        m_xRatio = (float)m_xsize / (float)neededXSize;
        m_yRatio = (float)m_ysize / (float)neededYSize;
#ifndef __APPLE__   
        m_coords[0].s = 0.f;
        m_coords[0].t = 0.f;
    
        m_coords[1].s = m_xRatio;
        m_coords[1].t = 0.f;
    
        m_coords[2].s = m_xRatio;
        m_coords[2].t = m_yRatio;
    
        m_coords[3].s = 0.f;
        m_coords[3].t = m_yRatio;
#else
        m_coords[3].s = 0.f;
        m_coords[3].t = 0.f;
    
        m_coords[2].s = m_xRatio;
        m_coords[2].t = 0.f;
    
        m_coords[1].s = m_xRatio;
        m_coords[1].t = m_yRatio;
    
        m_coords[0].s = 0.f;
        m_coords[0].t = m_yRatio;
#endif
    } else {
#ifndef __APPLE__
        m_coords[0].s = 0.f;
        m_coords[0].t = 0.f;
    
        m_coords[1].s = m_pixBlock.image.xsize;
        m_coords[1].t = 0.f;
    
        m_coords[2].s = m_pixBlock.image.xsize;
        m_coords[2].t = m_pixBlock.image.ysize;
    
        m_coords[3].s = 0.f;
        m_coords[3].t = m_pixBlock.image.ysize;
#else
        m_coords[3].s = 0.f;
        m_coords[3].t = 0.f;
    
        m_coords[2].s = m_pixBlock.image.xsize;
        m_coords[2].t = 0.f;
    
        m_coords[1].s = m_pixBlock.image.xsize;
        m_coords[1].t = m_pixBlock.image.ysize;
    
        m_coords[0].s = 0.f;
        m_coords[0].t = m_pixBlock.image.ysize;
#endif
    }
}

/////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_movieOS :: texFrame(GemState *state, int doit)
{
  GLenum target = GL_TEXTURE_2D;

  m_oldTexCoords=state->texCoords;
  m_oldNumCoords=state->numTexCoords;
  m_oldTexture  =state->texture;

  state->texture = 1;
  state->texCoords = m_coords;
  state->numTexCoords = 4;
  // enable to texture binding
  if (!GemMan::texture_rectangle_supported)	//tigital
  {
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, m_textureObj);
  }else{
    glEnable(GL_TEXTURE_RECTANGLE_EXT);
    glBindTexture(GL_TEXTURE_RECTANGLE_EXT, m_textureObj);

    target = GL_TEXTURE_RECTANGLE_EXT;
  }
  
  if (doit) {
    // if the size changed, then reset the texture
    if (m_pixBlock.image.csize != m_dataSize[0] ||
        m_pixBlock.image.xsize != m_dataSize[1] ||
        m_pixBlock.image.ysize != m_dataSize[2]) {
      m_dataSize[0] = m_pixBlock.image.csize;
      m_dataSize[1] = m_pixBlock.image.xsize;
      m_dataSize[2] = m_pixBlock.image.ysize;
      glTexImage2D(target, 0,
                   m_pixBlock.image.csize,
                   m_pixBlock.image.xsize,
                   m_pixBlock.image.ysize, 0,
                   m_pixBlock.image.format,
                   m_pixBlock.image.type,
                   m_pixBlock.image.data);
    }
    // okay, load in the actual pixel data
    glTexSubImage2D(target, 0,
                    0, 0,			// position
                    m_xsize,			// the x size of the data
                    m_ysize,			// the y size of the data
                    m_pixBlock.image.format,	// the format
                    m_pixBlock.image.type,	// the type
                    m_frame);		// the data + header offset
  }
}

/////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_movieOS :: postrender(GemState *state)
{
  state->texCoords   = m_oldTexCoords;
  state->numTexCoords= m_oldNumCoords;
  state->texture     = m_oldTexture;

  state->image       = m_oldImage;
  
  //  post("postrender");
  m_pixBlock.newimage = 0;

  if ( !GemMan::texture_rectangle_supported)
    glDisable(GL_TEXTURE_2D);
  else
    glDisable(GL_TEXTURE_RECTANGLE_EXT);
  
  if (m_numFrames>0 && m_reqFrame>m_numFrames){
    m_reqFrame = m_numFrames;
    outlet_bang(m_outEnd);
  }
}


/////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void pix_movieOS :: startRendering()
{
    glGenTextures(1, &m_textureObj);
    if ( ! GemMan::texture_rectangle_supported )
        glBindTexture(GL_TEXTURE_2D, m_textureObj);
    else
        glBindTexture(GL_TEXTURE_RECTANGLE_EXT, m_textureObj);
  
    setUpTextureState();
    
    m_pixBlock.newimage = 1;
    m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
}

/////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_movieOS :: stopRendering()
{
  if (m_textureObj) glDeleteTextures(1, &m_textureObj);
  m_textureObj = 0;
  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = 0;
}

/////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_movieOS :: setUpTextureState()
{
    if ( !GemMan::texture_rectangle_supported )				//tigital
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    } else {
        glTexParameterf(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_PRIORITY, 0.0);
        if (GLEW_APPLE_client_storage)
            glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, 1);
        else
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_RECTANGLE_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_movieOS :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_movieOS), gensym("pix_movie"), A_DEFSYM, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieOS::openMessCallback),
		  gensym("open"), A_SYMBOL, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieOS::changeImageCallback),
		  gensym("img_num"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_movieOS::autoCallback),
		  gensym("auto"), A_DEFFLOAT, A_NULL);
}

void pix_movieOS :: openMessCallback(void *data, t_symbol *filename)
{
    GetMyClass(data)->openMess(filename);
}

void pix_movieOS :: changeImageCallback(void *data, t_symbol *, int argc, t_atom *argv)
{
  //  GetMyClass(data)->changeImage((int)imgNum);
    GetMyClass(data)->changeImage((argc<1)?0:atom_getint(argv), (argc<2)?0:atom_getint(argv+1));
}

void pix_movieOS :: autoCallback(void *data, t_floatarg state)
{
  GetMyClass(data)->m_auto=!(!(int)state);
}

#endif /* GEM_FILMBACKEND defined */
