/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  render to offscreen buffer and make texture
   
  created 11/27/2005
   
  Copyright (c) 2005-2006 James Tittle II, tigital AT mac DOT com
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_CONTROLS_GEMFRAMEBUFFER_H_
#define _INCLUDE__GEM_CONTROLS_GEMFRAMEBUFFER_H_

#include "Base/GemBase.h"
#include <iostream>

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  gemframebuffer
    
  render to offscreen buffer and make texture

  DESCRIPTION
    
  "bang" - sends out a state list
    
  -----------------------------------------------------------------*/
class GEM_EXTERN gemframebuffer : public GemBase
{
  CPPEXTERN_HEADER(gemframebuffer, GemBase);

    public:

  //////////
  // Constructor
  gemframebuffer(t_symbol *format, t_symbol *type);

 protected:
       
  //////////
  // Destructor
  ~gemframebuffer(void);
             
  //////////
  // A render message
  void         render(GemState *state);
  void         postrender(GemState *state);
  void         initFBO(void);
  void         destroyFBO(void);
              
  //////////
  // Set up the modifying flags
  void         startRendering(void);
  //////////
  // Clean up the modifying flags
  void         stopRendering(void);

  // extension checks
  virtual bool isRunnable(void);

  //////////
  // change the size dimensions
  void         dimMess(int width, int height);
      
  ////////// 
  // format-message
  virtual void formatMess(std::string);
  virtual void typeMess(std::string);
      
  virtual void colorMess(t_symbol*,int argc, t_atom*argv);
  virtual void perspectiveMess(t_symbol*,int argc, t_atom*argv);

  virtual void rectangleMess(bool mode);
  virtual void texunitMess(int mode);

  virtual void fixFormat(GLenum wantedFormat);
  virtual void printInfo(void);
       
 private:
  GLboolean             m_haveinit, m_wantinit;
  GLuint      m_frameBufferIndex;
  GLuint      m_depthBufferIndex;
  GLuint      m_offScreenID;
  GLuint      m_texTarget;
  GLuint      m_texunit;
  int         m_width, m_height;
  bool        m_rectangle; // 1=TEXTURE_RECTANGLE_EXT, 0=TEXTURE_2D
  GLenum      m_canRectangle; // whichever rectangle formats are supported
  int         m_internalformat;
  int         m_format;
  GLenum      m_wantFormat;
  int         m_type;
  GLint       m_vp[4];
  GLfloat     m_color[4];
  GLfloat     m_FBOcolor[4];
  t_outlet   *m_outTexInfo;
  GLfloat     m_perspect[6];
    
  void        bangMess(void);
};

#endif   // for header file
