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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLRASTERPOS_I_H_
#define _INCLUDE__GEM_OPENGL_GEMGLRASTERPOS_I_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglRasterPos2i
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glRasterPos2i( GLint x, GLint y)"
 */

class GEM_EXTERN GEMglRasterPos2i : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglRasterPos2i, GemGLBase);

	public:
	  // Constructor
	  GEMglRasterPos2i (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglRasterPos2i ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	x;		// VAR
	  virtual void	xMess(t_float);	// FUN

	  GLint	y;		// VAR
	  virtual void	yMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 xMessCallback (void*, t_floatarg);
	  static void	 yMessCallback (void*, t_floatarg);
};
#endif // for header file
