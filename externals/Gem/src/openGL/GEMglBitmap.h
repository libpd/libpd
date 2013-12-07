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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLBITMAP_H_
#define _INCLUDE__GEM_OPENGL_GEMGLBITMAP_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglBitmap
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glBitmap( GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, GLubyte* bitmap)"
 */

class GEM_EXTERN GEMglBitmap : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglBitmap, GemGLBase);

	public:
	  // Constructor
	  GEMglBitmap (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglBitmap ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	xorig;		// VAR
	  virtual void	xorigMess(t_float);	// FUN

	  GLfloat	yorig;		// VAR
	  virtual void	yorigMess(t_float);	// FUN

	  GLfloat	xmove;		// VAR
	  virtual void	xmoveMess(t_float);	// FUN

	  GLfloat	ymove;		// VAR
	  virtual void	ymoveMess(t_float);	// FUN

	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 xorigMessCallback (void*, t_floatarg);
	  static void	 yorigMessCallback (void*, t_floatarg);
	  static void	 xmoveMessCallback (void*, t_floatarg);
	  static void	 ymoveMessCallback (void*, t_floatarg);
};
#endif // for header file
