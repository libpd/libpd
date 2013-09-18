////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital AT mac DOT com
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2005-2006 James Tittle II, tigital At mac DoT com
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "gemframebuffer.h"
#include <string.h>
#include "Gem/State.h"
#include "Gem/GLStack.h"

CPPEXTERN_NEW_WITH_TWO_ARGS(gemframebuffer, t_symbol *, A_DEFSYMBOL, t_symbol *, A_DEFSYMBOL);

/////////////////////////////////////////////////////////
//
// gemframebuffer
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
gemframebuffer :: gemframebuffer(t_symbol *format, t_symbol *type)
  : m_haveinit(false), m_wantinit(false), m_frameBufferIndex(0), m_depthBufferIndex(0),
    m_offScreenID(0), m_texTarget(GL_TEXTURE_2D), m_texunit(0),
    m_width(256), m_height(256),
    m_rectangle(false), 
    m_internalformat(GL_RGB8), m_format(GL_RGB), m_wantFormat(GL_RGB),
    m_type(GL_UNSIGNED_BYTE),
    m_outTexInfo(NULL)
{
  // create an outlet to send out texture info:
  //  - ID
  //  - width & height
  //  - format/type (ie. GL_TEXTURE_RECTANGLE or GL_TEXTURE_2D)
  //  - anything else?
  if(!m_outTexInfo)
    m_outTexInfo = outlet_new(this->x_obj, 0);

  m_FBOcolor[0] = 0.f;
  m_FBOcolor[1] = 0.f;
  m_FBOcolor[2] = 0.f;
  m_FBOcolor[3] = 0.f;

  m_perspect[0] = -1.f;
  m_perspect[1] = 1.f;	
  m_perspect[2] = -1.f;
  m_perspect[3] = 1.f;	
  m_perspect[4] = 1.f;
  m_perspect[5] = 20.f;	

  
  if(format && format->s_name && format!=gensym(""))
    formatMess(format->s_name);
  if(type   && type->s_name   && type  !=gensym(""))
    typeMess(type->s_name);
}

////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
gemframebuffer :: ~gemframebuffer()
{
  destroyFBO();
  outlet_free(m_outTexInfo);
}

////////////////////////////////////////////////////////
// extension check
//
/////////////////////////////////////////////////////////
bool gemframebuffer :: isRunnable() {
  if(!GLEW_VERSION_1_3) {
    error("openGL version 1.3 needed");
    return false;
  }

  if(GLEW_EXT_framebuffer_object) {
    m_wantinit=true;

    /* check rectangle possibilities */
    m_canRectangle=GL_TEXTURE_2D;
    if(GLEW_ARB_texture_rectangle)
      m_canRectangle=GL_TEXTURE_RECTANGLE_ARB;
    else if (GLEW_EXT_texture_rectangle)
      m_canRectangle=GL_TEXTURE_RECTANGLE_EXT;

    return true;
  }

  error("openGL framebuffer extension is not supported by this system");

  return false;
}

////////////////////////////////////////////////////////
// renderGL
//
/////////////////////////////////////////////////////////
void gemframebuffer :: render(GemState *state)
{
  gem::GLStack*stacks=NULL;
  if(state) {
    state->get(GemState::_GL_STACKS, stacks);
  }

  if(!m_width || !m_height) {
    error("width and height must be present!");
  }
 
  glActiveTexture(GL_TEXTURE0_ARB + m_texunit);

  if (m_wantinit)
    initFBO();
  
  // store the window viewport dimensions so we can reset them,
  // and set the viewport to the dimensions of our texture
  glGetIntegerv(GL_VIEWPORT, m_vp);
  glGetFloatv( GL_COLOR_CLEAR_VALUE, m_color );
	
  glBindTexture( m_texTarget, 0 );
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_frameBufferIndex);
  // Bind the texture to the frame buffer.
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT,
                            m_texTarget, m_offScreenID, 0);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT,
                               GL_RENDERBUFFER_EXT, m_depthBufferIndex);
  
  // debug yellow color
  // glClearColor( 1,1,0,0);
  glClearColor( m_FBOcolor[0], m_FBOcolor[1], m_FBOcolor[2], m_FBOcolor[3] );
  
  // Clear the buffers and reset the model view matrix.
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // We need a one-to-one mapping of pixels to texels in order to
  // ensure every element of our texture is processed. By setting our
  // viewport to the dimensions of our destination texture and drawing
  // a screen-sized quad (see below), we ensure that every pixel of our
  // texel is generated and processed in the fragment program.		
  glViewport(0,0, m_width, m_height);

  if(stacks) stacks->push(gem::GLStack::PROJECTION);
  glLoadIdentity();
  glFrustum( m_perspect[0],  m_perspect[1],  m_perspect[2],  m_perspect[3], m_perspect[4], m_perspect[5]);

  if(stacks) stacks->push(gem::GLStack::MODELVIEW);
  glLoadIdentity();
}

////////////////////////////////////////////////////////
// postRender
//
/////////////////////////////////////////////////////////
void gemframebuffer :: postrender(GemState *state)
{
  t_float w, h;
  gem::GLStack*stacks=NULL;
  if(state) {
    state->get(GemState::_GL_STACKS, stacks);
  }

  glActiveTexture(GL_TEXTURE0_ARB + m_texunit);

  if(m_texTarget== GL_TEXTURE_2D) {
    w=1.f;
    h=1.f;
  } else {
    w=static_cast<t_float>(m_width);
    h=static_cast<t_float>(m_height);
  }

  // GPGPU CONCEPT 4: Viewport-Sized Quad = Data Stream Generator.
  // In order to execute fragment programs, we need to generate pixels.
  // Drawing a quad the size of our viewport (see above) generates a
  // fragment for every pixel of our destination texture. Each fragment
  // is processed identically by the fragment program. Notice that in
  // the reshape() function, below, we have set the frustum to
  // orthographic, and the frustum dimensions to [-1,1].  Thus, our
  // viewport-sized quad vertices are at [-1,-1], [1,-1], [1,1], and
  // [-1,1]: the corners of the viewport.

  glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, 0 );
  glBindTexture( m_texTarget, m_offScreenID );

  if(stacks) stacks->pop(gem::GLStack::PROJECTION);
  if(stacks) stacks->pop(gem::GLStack::MODELVIEW);

  // reset to visible window's clear color
  glClearColor( m_color[0], m_color[1], m_color[2], m_color[3] );
  // reset to original viewport dimensions
  glViewport( m_vp[0], m_vp[1], m_vp[2], m_vp[3] );
  // now that the render is done,

  // send textureID, w, h, textureTarget to outlet
  t_atom ap[5];
  SETFLOAT(ap+0, static_cast<t_float>(m_offScreenID));
  SETFLOAT(ap+1, w);
  SETFLOAT(ap+2, h);
  SETFLOAT(ap+3, m_texTarget);
  SETFLOAT(ap+4, static_cast<t_float>(0.));
  outlet_list(m_outTexInfo, 0, 5, ap);
}

void gemframebuffer :: printInfo()
{
  std::string format, internalformat;
  switch(m_format) {
  case GL_YUV422_GEM: format="YUV"; break;
  case GL_RGB: format="RGB"; break;
  case GL_RGBA: format="RGBA"; break;
  case GL_BGRA: format="BGRA"; break;
  case GL_RGB_FLOAT32_ATI: format="RGB32"; break;
  default: format="<unknown>";
  }
  switch(m_internalformat) {
  case GL_YUV422_GEM: internalformat="YUV"; break;
  case GL_RGB: internalformat="RGB"; break;
  case GL_RGBA: internalformat="RGBA"; break;
  case GL_BGRA: internalformat="BGRA"; break;
  case GL_RGB_FLOAT32_ATI: internalformat="RGB32"; break;
  default: internalformat="<unknown>";
  }
  std::string rectangle;
  int rect=(m_rectangle?m_canRectangle:GL_TEXTURE_2D);
  if(GL_TEXTURE_2D==rect)   rectangle="2D";
  else if(GL_TEXTURE_RECTANGLE_ARB==rect)rectangle="RECTANGLE(ARB)";
  else if(GL_TEXTURE_RECTANGLE_EXT==rect)rectangle="RECTANGLE(EXT)";


  std::string type;
  switch(m_type) {
  case GL_UNSIGNED_BYTE: type="BYTE"; break;
  case GL_FLOAT        : type="FLOAT"; break;
  default              : type="unknown";
  }

  post("size: %dx%d", m_width, m_height);
  post("rectangle: %d -> %s", m_rectangle, rectangle.c_str());
  post("format: %s/%s [%d/%d]", format.c_str(), internalformat.c_str(), m_format, m_internalformat);
  post("type: %s [%d]", type.c_str(), m_type);
  post("texunit: %d", m_texunit);
}

/////////////////////////////////////////////////////////
// initFBO
//
/////////////////////////////////////////////////////////
void gemframebuffer :: initFBO()
{
  // clean up any existing FBO before creating a new one
  if(m_haveinit)
    destroyFBO();

  m_texTarget = (m_rectangle?m_canRectangle:GL_TEXTURE_2D);
  /* check supported formats */
  fixFormat(m_wantFormat);

  // Generate frame buffer object then bind it.
  glGenFramebuffersEXT(1, &m_frameBufferIndex);
  glGenRenderbuffersEXT(1, &m_depthBufferIndex);

  // Create the texture we will be using to render to.
  glGenTextures(1, &m_offScreenID);
  glBindTexture(m_texTarget, m_offScreenID);

  glTexImage2D( m_texTarget, 0, m_internalformat, m_width, m_height, 0, m_format, m_type, NULL );
  // 2.13.2006
  // GL_LINEAR causes fallback to software shader
  // so switching back to GL_NEAREST
  glTexParameteri(m_texTarget, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(m_texTarget, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint wrapmode = (GLEW_EXT_texture_edge_clamp)?GL_CLAMP_TO_EDGE:GL_CLAMP;

  glTexParameterf(m_texTarget, GL_TEXTURE_WRAP_S, wrapmode);
  glTexParameterf(m_texTarget, GL_TEXTURE_WRAP_T, wrapmode);
  
  // Initialize the render buffer.
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, m_depthBufferIndex);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, m_width, m_height);

  // Make sure we have not errors.
  GLenum status = glCheckFramebufferStatusEXT( GL_FRAMEBUFFER_EXT) ;
  if( status != GL_FRAMEBUFFER_COMPLETE_EXT )
    {
      switch(status) {                                          
      case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT");
        break;
      case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
        error("GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT");
        break;
      case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
        error("GL_FRAMEBUFFER_UNSUPPORTED_EXT");
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION_EXT:
        error("GL_INVALID_FRAMEBUFFER_OPERATION_EXT");
        break;
      default:
        error("Unknown ERROR %d", status);
      }
      return;
    }

  // Return out of the frame buffer.
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

  m_haveinit = true;
  m_wantinit = false;

  printInfo();
}

////////////////////////////////////////////////////////
// destroyFBO
//
/////////////////////////////////////////////////////////
void gemframebuffer :: destroyFBO()
{
  if(!GLEW_EXT_framebuffer_object)
    return;
  // Release all resources.
  if(m_depthBufferIndex) glDeleteRenderbuffersEXT(1, &m_depthBufferIndex);
  if(m_frameBufferIndex) glDeleteFramebuffersEXT(1, &m_frameBufferIndex);
  if(m_offScreenID) glDeleteTextures(1, &m_offScreenID);

  m_haveinit = false;
}

////////////////////////////////////////////////////////
// bangMess
//
/////////////////////////////////////////////////////////
void gemframebuffer :: bangMess()
{
  error("'bang' message not implemented");
}

////////////////////////////////////////////////////////
// startRendering
//
/////////////////////////////////////////////////////////
void gemframebuffer :: startRendering()
{
  m_wantinit = true;
}

////////////////////////////////////////////////////////
// stopRendering
//
/////////////////////////////////////////////////////////
void gemframebuffer :: stopRendering()
{
  destroyFBO();
}




////////////////////////////////////////////////////////
// dimMess
//
/////////////////////////////////////////////////////////
void gemframebuffer :: dimMess(int width, int height)
{
  if (width != m_width || height != m_height)
    {
      m_width = width;
      m_height = height;
      setModified();
    }
}

void gemframebuffer :: colorMess(t_symbol*s,int argc, t_atom*argv)
{
  float red=1., green=1., blue=1., alpha=1.;
  switch(argc) {
  case (4):
    alpha=atom_getfloat(argv+3);
  case (3):
    red =atom_getfloat(argv+0);
    green=atom_getfloat(argv+1);
    blue =atom_getfloat(argv+2);
    break;
  default:
    error("'color' message takes 3 (RGB) or 4 (RGBA) values");
    return;
  }

  m_FBOcolor[0] = red;
  m_FBOcolor[1] = green;
  m_FBOcolor[2] = blue;
  m_FBOcolor[3] = alpha;
    
  setModified();
}

void gemframebuffer :: perspectiveMess(t_symbol*s,int argc, t_atom*argv)
{
  switch(argc){
  case 6:
    m_perspect[0]=atom_getfloat(argv);
    m_perspect[1]=atom_getfloat(argv+1);
    m_perspect[2]=atom_getfloat(argv+2);
    m_perspect[3]=atom_getfloat(argv+3);
    m_perspect[4]=atom_getfloat(argv+4);
    m_perspect[5]=atom_getfloat(argv+5);

    setModified();
    break;
  default:
    error("\"perspec\" expects 6 values for frustum - left, right, bottom, top, near, far");
  }
}



/* needs to be called with a valid context */
void gemframebuffer :: fixFormat(GLenum wantFormat)
{
   m_type = GL_UNSIGNED_BYTE;

  if(wantFormat == GL_RGB_FLOAT32_ATI && !GLEW_ATI_texture_float) {
    wantFormat =  GL_RGB;
  }

  switch(wantFormat) {
  default:
    post("using default format");
  case GL_RGB:
    m_internalformat=m_format=GL_RGB;
    break;
  case  GL_RGB_FLOAT32_ATI:
    m_internalformat = GL_RGB_FLOAT32_ATI;
    m_format = GL_RGB;
    break;
  case GL_RGBA:
    m_internalformat = GL_RGBA;
    m_format = GL_RGBA;
    break;
  case GL_YUV422_GEM:
    m_format=GL_YUV422_GEM;
    m_internalformat=GL_RGB8;
    break;
  }

#ifdef __APPLE__
  switch(wantFormat) {
  case  GL_RGB_FLOAT32_ATI:
  m_format = GL_BGR;
  break;
  case GL_RGBA:
    m_format = GL_BGRA;
    break;
  case GL_YUV422_GEM:
    m_type = GL_UNSIGNED_SHORT_8_8_REV_APPLE;
    break;
  default:
    break;
  }
#endif
}


void gemframebuffer :: formatMess(std::string format)
{
  GLenum tmp_format=0;
  if("YUV"==format) {
    tmp_format = GL_YUV422_GEM;
  } else if ("RGB"==format) {
    tmp_format = GL_RGB;
  } else if ("RGBA"==format) {
    tmp_format = GL_RGBA;
  } else if ("RGB32"==format) {
      tmp_format =  GL_RGB_FLOAT32_ATI;
  }

  if(tmp_format)
    m_wantFormat=tmp_format;
  setModified();
}

void gemframebuffer :: typeMess(std::string type)
{
  if("FLOAT"==type) {
    m_type = GL_FLOAT;
  } else {
    type="BYTE";
    m_type=GL_UNSIGNED_BYTE;
  }
  // changed type, so we need to rebuild the FBO
  setModified();
}

void gemframebuffer :: rectangleMess(bool rectangle)
{
  m_rectangle=rectangle;
  setModified();
}
void gemframebuffer :: texunitMess(int unit) {
  m_texunit=static_cast<GLuint>(unit);
}



////////////////////////////////////////////////////////
// static member function
//
////////////////////////////////////////////////////////
void gemframebuffer :: obj_setupCallback(t_class *classPtr)
{
  CPPEXTERN_MSG0(classPtr, "bang",   bangMess);
  CPPEXTERN_MSG (classPtr, "color",  colorMess);
  CPPEXTERN_MSG (classPtr, "perspec",  perspectiveMess);
  CPPEXTERN_MSG2(classPtr, "dimen",  dimMess, int, int);
  CPPEXTERN_MSG1(classPtr, "format", formatMess, std::string);
  CPPEXTERN_MSG1(classPtr, "type",   typeMess, std::string);
  CPPEXTERN_MSG1(classPtr, "rectangle", rectangleMess, bool);
  CPPEXTERN_MSG1(classPtr, "texunit",   texunitMess, int);

  /* legacy */
  CPPEXTERN_MSG2(classPtr, "dim",    dimMess, int, int);
  CPPEXTERN_MSG1(classPtr, "mode",   rectangleMess, bool);
}
