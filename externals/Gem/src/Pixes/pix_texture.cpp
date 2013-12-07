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
//    Copyright (c) 2002-2006 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "pix_texture.h"

#include "Gem/Settings.h"
#include "Gem/Image.h"
#include <string.h>

#ifdef debug
# undef debug
#endif

//#define DEBUG_ME

#ifdef DEBUG_ME
# define debug post
#else
# define debug
#endif

CPPEXTERN_NEW(pix_texture);

/////////////////////////////////////////////////////////
//
// pix_texture
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_texture :: pix_texture()
  : m_textureOnOff(1),
    m_textureQuality(GL_LINEAR), m_repeat(GL_REPEAT), m_doRepeat(GL_REPEAT),
    m_didTexture(false), m_rebuildList(false),
    m_textureObj(0),
    m_extTextureObj(0), m_extWidth(1.), m_extHeight(1.), m_extType(GL_TEXTURE_2D),
    m_extUpsidedown(false),
    m_realTextureObj(0),
    m_oldTexCoords(NULL), m_oldNumCoords(0), m_oldTexture(0),
    m_textureType( GL_TEXTURE_2D ),
    m_rectangle(0), m_env(GL_MODULATE),
    m_clientStorage(0), //have to do this due to texture corruption issues
    m_yuv(1),
    m_texunit(0),
    m_numTexUnits(0),
    m_numPbo(0), m_curPbo(0), m_pbo(NULL),
    m_upsidedown(false)
{
  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
  m_buffer.xsize = m_buffer.ysize = m_buffer.csize = -1;
  m_buffer.data = NULL;

  //rectangle textures by default only for OSX since there are too many busted drivers in use on Windows and Linux
#ifdef __APPLE__
  m_rectangle = 1;  //default to the fastest mode for systems that support it
  m_textureType = GL_TEXTURE_RECTANGLE_ARB;
#endif

  int ival=1;
  GemSettings::get("texture.repeat", ival);
  repeatMess(ival);

  ival=1;
  GemSettings::get("texture.quality", ival);
  textureQuality(ival);

  GemSettings::get("texture.rectangle", m_rectangle);
  GemSettings::get("texture.pbo", m_numPbo);

  // create an inlet to receive external texture IDs
  m_inTexID  = inlet_new(this->x_obj, &this->x_obj->ob_pd, &s_float, gensym("extTexture"));

  // create an outlet to send texture ID
  m_outTexID = outlet_new(this->x_obj, &s_float);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_texture :: ~pix_texture()
{
  if(m_inTexID) inlet_free (m_inTexID);
  if(m_outTexID)outlet_free(m_outTexID);

  m_inTexID=NULL;
  m_outTexID=NULL;
}

////////////////////////////////////////////////////////
// setUpTextureState
//
/////////////////////////////////////////////////////////
void pix_texture :: setUpTextureState() {
  m_doRepeat=m_repeat;
  if (m_rectangle && m_canRectangle){
    if ( m_textureType ==  GL_TEXTURE_RECTANGLE_ARB || m_textureType == GL_TEXTURE_RECTANGLE_EXT)
      {
        glTexParameterf(m_textureType, GL_TEXTURE_PRIORITY, 0.0f);
        // JMZ: disabled the following, as rectangle-textures are clamped anyhow
        // JMZ: and normalized ones, lose their setting
        // TIGITAL: this is necessary on osx, at least with non-powerof2 textures!
        //			otherwise, weird texturing occurs (looks similar to pix_refraction)
        // NPOT: GL_CLAMP, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_BORDER
        // POT:  above plus GL_REPEAT, GL_MIRRORED_REPEAT
        m_doRepeat = GL_CLAMP_TO_EDGE;
        debug("using rectangle texture");
      }
  }

  if (GLEW_APPLE_client_storage){
    if(m_clientStorage){
      glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
      debug("using client storage");
    } else {
      glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_FALSE);
      debug("not using client storage");
    }
  } else
    glPixelStoref(GL_UNPACK_ALIGNMENT, 1);

  glTexParameterf(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
  glTexParameterf(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_doRepeat);
  glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_doRepeat);
}

////////////////////////////////////////////////////////
// setTexCoords
//
// upsidedown is derived from the imageStruct.upsidedown
// use this when loading images...
//
/////////////////////////////////////////////////////////
inline void setTexCoords(TexCoord *coords, float xRatio, float yRatio, GLboolean upsidedown=false){
  if(!upsidedown){
    coords[0].s = 0.f;
    coords[0].t = 0.f;
    coords[1].s = xRatio;
    coords[1].t = 0.f;
    coords[2].s = xRatio;
    coords[2].t = yRatio;
    coords[3].s = 0.f;
    coords[3].t = yRatio;
  } else {
    coords[3].s = 0.f;
    coords[3].t = 0.f;
    coords[2].s = xRatio;
    coords[2].t = 0.f;
    coords[1].s = xRatio;
    coords[1].t = yRatio;
    coords[0].s = 0.f;
    coords[0].t = yRatio;
  }
}

static inline void tex2state(GemState*state, TexCoord*coords, int size) {
  state->set(GemState::_GL_TEX_COORDS, coords);
  state->set(GemState::_GL_TEX_NUMCOORDS, size);
}


////////////////////////////////////////////////////////
// extension check
//
/////////////////////////////////////////////////////////
bool pix_texture :: isRunnable(void) {
  /* for simplicity's sake, i have dropped support for very old openGL-versions */
  if(!GLEW_VERSION_1_1) {
    error("need at least openGL-1.1 for texturing! refusing to work");
    return false;
  }

  m_numTexUnits=0;
  if(GLEW_ARB_multitexture)
    glGetIntegerv( GL_MAX_TEXTURE_UNITS_ARB, &m_numTexUnits );

  int wantRectangle=1;
  GemSettings::get("texture.rectangle", wantRectangle);

  m_canRectangle=0;
  if(wantRectangle) {
    if(GLEW_ARB_texture_rectangle)
      m_canRectangle=2;
    else if (GLEW_EXT_texture_rectangle)
      m_canRectangle=1;
  }

  return true;
}

void pix_texture :: pushTexCoords(GemState*state) {
  state->get(GemState::_GL_TEX_COORDS, m_oldTexCoords);
  state->get(GemState::_GL_TEX_NUMCOORDS, m_oldNumCoords);
  state->get(GemState::_GL_TEX_TYPE, m_oldTexture);
}

void pix_texture :: popTexCoords(GemState*state) {
  tex2state(state, m_oldTexCoords, m_oldNumCoords);
  state->set(GemState::_GL_TEX_TYPE, m_oldTexture);
}


void pix_texture :: sendExtTexture(GLuint texobj, GLfloat xRatio, GLfloat yRatio, GLint texType, GLboolean upsidedown) {
  // send textureID to outlet
  if(texobj){
    t_atom ap[5];
    SETFLOAT(ap, (t_float)texobj);
    SETFLOAT(ap+1, (t_float)xRatio);
    SETFLOAT(ap+2, (t_float)yRatio);
    SETFLOAT(ap+3, (t_float)texType);
    SETFLOAT(ap+4, (t_float)upsidedown);
    outlet_list(m_outTexID, &s_list, 5, ap);
  }
}

////////////////////////////////////////////////////////
// render
//
/////////////////////////////////////////////////////////
void pix_texture :: render(GemState *state) {
  m_didTexture=false;
  pushTexCoords(state);

  if(!m_textureOnOff)return;
  if(!state)return;

  GLboolean upsidedown=false;
  GLboolean normalized=true;

  int texType = m_textureType;
  int x_2=1, y_2=1;
  GLboolean useExternalTexture=false;
  int do_rectangle = (m_rectangle)?m_canRectangle:0;
  int newfilm = 0;
  pixBlock*img=NULL;


  state->get(GemState::_PIX, img);
  if(img)
    newfilm = img->newfilm;

  if (!img || !img->image.data){
    if(m_extTextureObj>0) {
      useExternalTexture= true;
      m_rebuildList     = false;
      m_textureObj      = m_extTextureObj;
      if(m_extType)m_textureType=m_extType;
      texType=m_textureType;
      upsidedown=m_extUpsidedown;
      m_xRatio=m_extWidth;
      m_yRatio=m_extHeight;
      m_upsidedown=upsidedown;
    } else
      /* neither do we have an image nor an external texture */
      return;
  }
  tex2state(state, m_coords, 4);

  if(!useExternalTexture){
    upsidedown = img->image.upsidedown;
    if (img->newimage) m_rebuildList = true;

    m_imagebuf.xsize =img->image.xsize;
    m_imagebuf.ysize =img->image.ysize;
    m_imagebuf.csize =img->image.csize;
    m_imagebuf.format=img->image.format;
    m_imagebuf.type  =img->image.type;
    m_imagebuf.data  =img->image.data;

    x_2 = powerOfTwo(m_imagebuf.xsize);
    y_2 = powerOfTwo(m_imagebuf.ysize);

    normalized = ((m_imagebuf.xsize==x_2) && (m_imagebuf.ysize==y_2));

    debug("normalized=%d\t%d - %d\t%d - %d", normalized, m_imagebuf.xsize, x_2, m_imagebuf.ysize, y_2);

    switch(do_rectangle) {
    case 2:
      m_textureType = GL_TEXTURE_RECTANGLE_ARB;
      debug("using mode 1:GL_TEXTURE_RECTANGLE_ARB");
      normalized = 0;
      break;
    case 1:
      m_textureType = GL_TEXTURE_RECTANGLE_EXT;
      debug("using mode 1:GL_TEXTURE_RECTANGLE_EXT");
      normalized = 0;
      break;
    default:
      m_textureType = GL_TEXTURE_2D;
      debug("using mode 0:GL_TEXTURE_2D");
      normalized = 0;
      break;
    }

    debug("normalized=%d", normalized);
  }

  if (m_textureType!=texType){
    debug("texType != m_textureType");
    stopRendering();startRendering();
  }

  if(GLEW_VERSION_1_3) {
    glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
  }
  glEnable(m_textureType);
  glBindTexture(m_textureType, m_textureObj);

  if(useExternalTexture) {
    glTexParameterf(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
    glTexParameterf(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
  }

  if ((!useExternalTexture)&&newfilm ){
    //  tigital:  shouldn't we also allow TEXTURE_2D here?
    if(NULL!=glTextureRangeAPPLE) {
      if ( GLEW_APPLE_texture_range ){
        if(glTextureRangeAPPLE == NULL) {
          glTextureRangeAPPLE( m_textureType,
                               m_imagebuf.xsize * m_imagebuf.ysize * m_imagebuf.csize,
                               m_imagebuf.data );
          debug("using glTextureRangeAPPLE()");
        }else{
          glTextureRangeAPPLE( m_textureType, 0, NULL );
        }
      }
    }

    /* hmm, GL_TEXTURE_STORAGE_HINT_APPLE throws a GL-error on linux (and probably on w32 too)
     * how to do a run-time check for it?
     *
     * according to http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_texturedata/chapter_10_section_2.html
     * this seems to be a part of the texture_range extension, so we check for that!
     */
    if(GLEW_APPLE_texture_range)
       glTexParameteri( m_textureType, GL_TEXTURE_STORAGE_HINT_APPLE, GL_STORAGE_SHARED_APPLE );
    // GL_STORAGE_SHARED_APPLE -  AGP texture path
    // GL_STORAGE_CACHED_APPLE - VRAM texture path
    // GL_STORAGE_PRIVATE_APPLE - normal texture path
    if(m_clientStorage) glPixelStorei(GL_UNPACK_CLIENT_STORAGE_APPLE, GL_TRUE);
  }



  /* here comes the work: a new image has to be transfered from main memory to GPU and attached to a texture object */

  if (m_rebuildList) {
    // if YUV is not supported on this platform, we have to convert it to RGB
    //(skip Alpha since it isnt used)
    const bool do_yuv = m_yuv && GLEW_APPLE_ycbcr_422;
    if (!do_yuv && m_imagebuf.format == GL_YUV422_GEM){
      m_imagebuf.format=GL_RGB;
      m_imagebuf.csize=3;
      m_imagebuf.reallocate();
      m_imagebuf.fromYUV422(img->image.data);
    }
    if (normalized) {
      m_buffer.xsize = m_imagebuf.xsize;
      m_buffer.ysize = m_imagebuf.ysize;
      m_buffer.csize  = m_imagebuf.csize;
      m_buffer.format = m_imagebuf.format;
      m_buffer.type   = m_imagebuf.type;
      m_buffer.reallocate();
      m_xRatio=1.0;
      m_yRatio=1.0;
      m_upsidedown=upsidedown;

      tex2state(state, m_coords, 4);
      if (m_buffer.csize != m_dataSize[0] ||
          m_buffer.xsize != m_dataSize[1] ||
          m_buffer.ysize != m_dataSize[2]){
        m_dataSize[0] = m_buffer.csize;
        m_dataSize[1] = m_buffer.xsize;
        m_dataSize[2] = m_buffer.ysize;

      }
      //if the texture is a power of two in size then there is no need to subtexture
      glTexImage2D(m_textureType, 0,
                   m_imagebuf.csize,
                   m_imagebuf.xsize,
                   m_imagebuf.ysize, 0,
                   m_imagebuf.format,
                   m_imagebuf.type,
                   m_imagebuf.data);

    } else { // !normalized
      m_xRatio = (float)m_imagebuf.xsize;
      m_yRatio = (float)m_imagebuf.ysize;
      if ( !do_rectangle ) {
        m_xRatio /= (float)x_2;
        m_yRatio /= (float)y_2;
        m_buffer.xsize = x_2;
        m_buffer.ysize = y_2;
      } else {
        m_buffer.xsize = m_imagebuf.xsize;
        m_buffer.ysize = m_imagebuf.ysize;
      }

      m_buffer.csize  = m_imagebuf.csize;
      m_buffer.format = m_imagebuf.format;
      m_buffer.type   = m_imagebuf.type;
      m_buffer.reallocate();
      m_upsidedown=upsidedown;
      tex2state(state, m_coords, 4);

      if (m_buffer.csize != m_dataSize[0] ||
          m_buffer.xsize != m_dataSize[1] ||
          m_buffer.ysize != m_dataSize[2]){
        newfilm = 1;

      } //end of loop if size has changed

      // okay, load in the actual pixel data

      //when doing rectangle textures the buffer changes after every film is loaded this call makes sure the
      //texturing is updated as well to prevent crashes
      if(newfilm) {
        m_dataSize[0] = m_buffer.csize;
        m_dataSize[1] = m_buffer.xsize;
        m_dataSize[2] = m_buffer.ysize;

        if (m_buffer.format == GL_YUV422_GEM && !m_rectangle)m_buffer.setBlack();

        if(m_numPbo>0) {
          if(GLEW_ARB_pixel_buffer_object) {
            if(m_pbo) {
              delete[]m_pbo;
              m_pbo=NULL;
            }
            m_pbo=new GLuint[m_numPbo];
            glGenBuffersARB(m_numPbo, m_pbo);
            int i=0;
            for(i=0; i<m_numPbo; i++) {
              glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_pbo[i]);
              glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB, 
                              m_buffer.xsize*m_buffer.ysize*m_buffer.csize,
                              0, GL_STREAM_DRAW_ARB);
            }
            glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);
 
          } else {
            logpost(NULL, 5, "PBOs not supported! disabling");
            m_numPbo=0;

          }
        }

        //this is for dealing with power of 2 textures which need a buffer that's 2^n
        if ( !do_rectangle ) {
          glTexImage2D(	m_textureType, 0,
                        //m_buffer.csize,
                        GL_RGBA,
                        m_buffer.xsize,
                        m_buffer.ysize, 0,
                        m_buffer.format,
                        m_buffer.type,
                        m_buffer.data);

          debug("TexImage2D non rectangle");
        } else {//this deals with rectangle textures that are h*w
          glTexImage2D(m_textureType, 0,
                       //  m_buffer.csize,
                       GL_RGBA,
                       m_imagebuf.xsize,
                       m_imagebuf.ysize, 0,
                       m_imagebuf.format,
                       m_imagebuf.type,
                       m_imagebuf.data);
          debug("TexImage2D  rectangle");
        }

        // just to make sure...
        img->newfilm = 0;
      }

      if(m_pbo) {
        m_curPbo=(m_curPbo+1)%m_numPbo;
        int index=m_curPbo;
        int nextIndex=(m_curPbo+1)%m_numPbo;

        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_pbo[index]);
        glTexSubImage2D(m_textureType, 0, 
                        0, 0, 
                        m_imagebuf.xsize, 
                        m_imagebuf.ysize, 
                        m_imagebuf.format, 
                        m_imagebuf.type, 
                        NULL); /* <-- that's the key */


        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, m_pbo[nextIndex]);
        glBufferDataARB(GL_PIXEL_UNPACK_BUFFER_ARB,  m_imagebuf.xsize * m_imagebuf.ysize * m_imagebuf.csize, 0, GL_STREAM_DRAW_ARB);

        GLubyte* ptr = (GLubyte*)glMapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, GL_WRITE_ONLY_ARB);
        if(ptr)
          {
            // update data off the mapped buffer
            memcpy(ptr, m_imagebuf.data,  m_imagebuf.xsize * m_imagebuf.ysize * m_imagebuf.csize);
            glUnmapBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB); // release pointer to mapping buffer
          }

        /* unbind the current buffer */
        glBindBufferARB(GL_PIXEL_UNPACK_BUFFER_ARB, 0);

      } else {
        glTexSubImage2D(m_textureType, 0,
                        0, 0,				// position
                        m_imagebuf.xsize,
                        m_imagebuf.ysize,
                        m_imagebuf.format,
                        m_imagebuf.type,
                        m_imagebuf.data);
      }
    }
  } // rebuildlist

  setTexCoords(m_coords, m_xRatio, m_yRatio, m_upsidedown);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, m_env);

  /* cleanup */
  m_rebuildList = false;
  m_didTexture=true;

  state->set(GemState::_GL_TEX_UNITS, m_numTexUnits);

  // if we are using rectangle textures, this is a way to inform the downstream objects
  // (this is important for things like [pix_coordinate]

  // we don't use switch/case as _ARB and _EXT might be the same...
  if(m_textureType==GL_TEXTURE_RECTANGLE_ARB || m_textureType==GL_TEXTURE_RECTANGLE_EXT) {
    state->set(GemState::_GL_TEX_TYPE, 2);
  } else {
    state->set(GemState::_GL_TEX_TYPE, 1);
  }

  sendExtTexture(m_textureObj, m_xRatio, m_yRatio, m_textureType, upsidedown);
}

////////////////////////////////////////////////////////
// postrender
//
/////////////////////////////////////////////////////////
void pix_texture :: postrender(GemState *state){
  popTexCoords(state);

  if (m_didTexture){
    if(GLEW_VERSION_1_3) {
      glActiveTexture(GL_TEXTURE0_ARB + m_texunit);  //needed?
    }
    glDisable(m_textureType);

    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

    // to avoid matrix stack confusion, we reset the upstream texunit to 0
    if(GLEW_VERSION_1_3) {
      glActiveTexture(GL_TEXTURE0_ARB);
    }
  }

}

////////////////////////////////////////////////////////
// startRendering
//
////////////////////////////////////////////////////////
void pix_texture :: startRendering()
{
  glGenTextures(1, &m_realTextureObj); // this crashes sometimes!!!! (jmz)
  if(GLEW_VERSION_1_3) {
    glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
  }
  glBindTexture(m_textureType, m_realTextureObj);
  m_textureObj=m_realTextureObj;
  setUpTextureState();

  m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;

  if (!m_realTextureObj)	{
    error("Unable to allocate texture object");
    return;
  }
}

////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void pix_texture :: stopRendering()
{
  if(m_realTextureObj) {
    glDeleteTextures(1, &m_realTextureObj);

    m_realTextureObj = 0;
    m_dataSize[0] = m_dataSize[1] = m_dataSize[2] = -1;
  }

  if(m_pbo) {
    glDeleteBuffersARB(m_numPbo, m_pbo);
    delete[]m_pbo;
    m_pbo=NULL;
  }

}


////////////////////////////////////////////////////////
// textureQuality
//
/////////////////////////////////////////////////////////
void pix_texture :: setModified()
{
  m_rebuildList=true;
  GemBase::setModified();
}

////////////////////////////////////////////////////////
// textureOnOff
//
/////////////////////////////////////////////////////////
void pix_texture :: textureOnOff(int on)
{
  m_textureOnOff = on;
  setModified();
}

/////////////////////////////////////////////////////////
// textureQuality
//
/////////////////////////////////////////////////////////
void pix_texture :: textureQuality(int type)
{
  if (type)
    m_textureQuality = GL_LINEAR;
  else
    m_textureQuality = GL_NEAREST;

  if (m_textureObj) {
    if(GLEW_VERSION_1_3) {
      glActiveTexture(GL_TEXTURE0_ARB + m_texunit);
    }
    glBindTexture(m_textureType, m_textureObj);
    glTexParameterf(m_textureType, GL_TEXTURE_MAG_FILTER, m_textureQuality);
    glTexParameterf(m_textureType, GL_TEXTURE_MIN_FILTER, m_textureQuality);
  }
  setModified();
}

////////////////////////////////////////////////////////
// textureQuality
//
/////////////////////////////////////////////////////////
void pix_texture :: textureRectangle(int rect)
{
  m_rectangle=rect;
  if (m_rectangle)
    post("using mode 1: TEXTURE_RECTANGLE");
  else
    post("using mode 0: TEXTURE_2D");

  setModified();
}

////////////////////////////////////////////////////////
// texture repeat message
//
/////////////////////////////////////////////////////////
void pix_texture :: repeatMess(int type)
{
  if (type)
    m_repeat = GL_REPEAT;
  else {
    if(GLEW_EXT_texture_edge_clamp)
      m_repeat = GL_CLAMP_TO_EDGE;
    else
      m_repeat = GL_CLAMP;
  }

  if ( m_textureType ==  GL_TEXTURE_RECTANGLE_ARB || m_textureType == GL_TEXTURE_RECTANGLE_EXT)
    m_doRepeat=GL_CLAMP_TO_EDGE;
  else
    m_doRepeat=m_repeat;

  if (m_textureObj) {
    if(GLEW_VERSION_1_1) {
      glBindTexture(m_textureType, m_textureObj);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_S, m_doRepeat);
      glTexParameterf(m_textureType, GL_TEXTURE_WRAP_T, m_doRepeat);
    } else {
      glBindTextureEXT(m_textureType, m_textureObj);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_S, m_doRepeat);
      glTexParameteri(m_textureType, GL_TEXTURE_WRAP_T, m_doRepeat);
    }
  }
  setModified();
}

////////////////////////////////////////////////////////
// texture environment mode
//
/////////////////////////////////////////////////////////
void pix_texture :: envMess(int num)
{
  switch (num) {
  case 0:
    m_env = GL_REPLACE;
    break;
  case 1:
    m_env = GL_DECAL;
    break;
  case 2:
    m_env = GL_BLEND;
    break;
  case 3:
    m_env = GL_ADD;
    break;
  case 4:
    m_env = GL_COMBINE;
    break;
  default:
    m_env = GL_MODULATE;
  }
  setModified();
}



////////////////////////////////////////////////////////
// Pixel Buffer Object message
//
/////////////////////////////////////////////////////////
void pix_texture :: pboMess(int num)
{
  if(num<0) {
    return;
  }

  if(m_pbo) {
    glDeleteBuffersARB(m_numPbo, m_pbo);
    delete[]m_pbo;
    m_pbo=NULL;
    m_numPbo=0;
  }

  m_numPbo=num;
  setModified();
}


////////////////////////////////////////////////////////
// static member functions
//
/////////////////////////////////////////////////////////
void pix_texture :: obj_setupCallback(t_class *classPtr)
{
  class_addfloat(classPtr, reinterpret_cast<t_method>(&pix_texture::floatMessCallback));
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::textureMessCallback),
                  gensym("quality"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::repeatMessCallback),
                  gensym("repeat"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::envMessCallback),
                  gensym("env"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::modeCallback),
                  gensym("mode"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::rectangleCallback),
                  gensym("rectangle"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::clientStorageCallback),
                  gensym("client_storage"), A_FLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::yuvCallback),
                  gensym("yuv"), A_FLOAT, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::extTextureCallback),
                  gensym("extTexture"), A_GIMME, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::texunitCallback),
                  gensym("texunit"), A_FLOAT, A_NULL);

  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_texture::pboCallback),
                  gensym("pbo"), A_FLOAT, A_NULL);

  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_texture), gensym("pix_texture2"), A_NULL);
}
void pix_texture :: floatMessCallback(void *data, float n)
{
  GetMyClass(data)->textureOnOff((int)n);
}
void pix_texture :: textureMessCallback(void *data, t_floatarg quality)
{
  GetMyClass(data)->textureQuality((int)quality);
}
void pix_texture :: repeatMessCallback(void *data, t_floatarg repeat)
{
  GetMyClass(data)->repeatMess((int)repeat);
}
void pix_texture :: envMessCallback(void *data, t_floatarg num )
{
  GetMyClass(data)->envMess((int) num);
}
void pix_texture :: modeCallback(void *data, t_floatarg rectangle)
{
  GetMyClass(data)->error("'mode' message is deprecated; please use 'rectangle' instead");
  GetMyClass(data)->textureRectangle((int)rectangle);
}
void pix_texture :: rectangleCallback(void *data, t_floatarg rectangle)
{
  GetMyClass(data)->textureRectangle((int)rectangle);
}

void pix_texture :: clientStorageCallback(void *data, t_floatarg do_clientstorage)
{
  GetMyClass(data)->m_clientStorage=((int)do_clientstorage);
}

void pix_texture :: yuvCallback(void *data, t_floatarg do_yuv)
{
  GetMyClass(data)->m_yuv=((int)do_yuv);
}

void pix_texture :: extTextureCallback(void *data, t_symbol*s, int argc, t_atom*argv)
{
  int index=5;
  switch(argc){
  case 5:
    if(A_FLOAT!=argv[4].a_type)break;
    GetMyClass(data)->m_extUpsidedown=atom_getint(argv+4);
  case 4:
    index=4;
    if(A_FLOAT!=argv[3].a_type)break;
    GetMyClass(data)->m_extType=atom_getint(argv+3);
  case 3:
    index=3;
    if(A_FLOAT!=argv[2].a_type)break;
    index=2;
    if(A_FLOAT!=argv[1].a_type)break;
    GetMyClass(data)->m_extWidth =atom_getfloat(argv+1);
    GetMyClass(data)->m_extHeight=atom_getfloat(argv+2);
  case 1:
    index=1;
    if(A_FLOAT!=argv[0].a_type)break;
    GetMyClass(data)->m_extTextureObj=atom_getint(argv+0);
    index=0;
    return;
  default:
    GetMyClass(data)->error("arguments: <texId> [<width> <height> [<type> [<upsidedown>]]]");
    return;
  }
  if(index)
    GetMyClass(data)->error("invalid type of argument #%d", index);


}
void pix_texture :: texunitCallback(void *data, t_floatarg unit)
{
  GetMyClass(data)->m_texunit=(int)unit;
}

void pix_texture :: pboCallback(void *data, t_floatarg numpbo)
{
  GetMyClass(data)->pboMess((int)numpbo);
}
