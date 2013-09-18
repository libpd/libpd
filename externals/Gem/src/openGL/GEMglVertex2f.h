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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLVERTEX_F_H_
#define _INCLUDE__GEM_OPENGL_GEMGLVERTEX_F_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglVertex2f
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glVertex2f( GLfloat x, GLfloat y)"
 */

class GEM_EXTERN GEMglVertex2f : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglVertex2f, GemGLBase);

	public:
	  // Constructor
	  GEMglVertex2f (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglVertex2f ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	x;		// VAR
	  virtual void	xMess(t_float);	// FUN

	  GLfloat	y;		// VAR
	  virtual void	yMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 xMessCallback (void*, t_floatarg);
	  static void	 yMessCallback (void*, t_floatarg);
};
#endif // for header file
