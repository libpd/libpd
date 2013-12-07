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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLSCALED_H_
#define _INCLUDE__GEM_OPENGL_GEMGLSCALED_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglScaled
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glScaled( GLdouble x, GLdouble y, GLdouble z)"
 */

class GEM_EXTERN GEMglScaled : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglScaled, GemGLBase);

	public:
	  // Constructor
	  GEMglScaled (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglScaled ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	x;		// VAR
	  virtual void	xMess(t_float);	// FUN

	  GLdouble	y;		// VAR
	  virtual void	yMess(t_float);	// FUN

	  GLdouble	z;		// VAR
	  virtual void	zMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 xMessCallback (void*, t_floatarg);
	  static void	 yMessCallback (void*, t_floatarg);
	  static void	 zMessCallback (void*, t_floatarg);
};
#endif // for header file
