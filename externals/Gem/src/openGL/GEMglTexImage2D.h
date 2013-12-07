 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
  *	zmoelnig@iem.kug.ac.at
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  *  this file has been generated...
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLTEXIMAGE_D_H_
#define _INCLUDE__GEM_OPENGL_GEMGLTEXIMAGE_D_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglTexImage2D
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glTexImage2D( GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid *pixels)"
 */

class GEM_EXTERN GEMglTexImage2D : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglTexImage2D, GemGLBase);

	public:
	  // Constructor
	  GEMglTexImage2D (int,t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglTexImage2D ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLint	level;		// VAR
	  virtual void	levelMess(t_float);	// FUN

	  GLint	xoffset;		// VAR
	  virtual void	xoffsetMess(t_float);	// FUN

	  GLint	yoffset;		// VAR
	  virtual void	yoffsetMess(t_float);	// FUN

	  GLsizei	width;		// VAR
	  virtual void	widthMess(t_float);	// FUN

	  GLsizei	height;		// VAR
	  virtual void	heightMess(t_float);	// FUN

	private:

	// we need some inlets
	  t_inlet *m_inlet[5];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 levelMessCallback (void*, t_floatarg);
	  static void	 xoffsetMessCallback (void*, t_floatarg);
	  static void	 yoffsetMessCallback (void*, t_floatarg);
	  static void	 widthMessCallback (void*, t_floatarg);
	  static void	 heightMessCallback (void*, t_floatarg);
};
#endif // for header file
