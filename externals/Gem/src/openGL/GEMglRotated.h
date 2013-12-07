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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLROTATED_H_
#define _INCLUDE__GEM_OPENGL_GEMGLROTATED_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglRotated
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glRotated( GLdouble angle, GLdouble x, GLdouble y, GLdouble z)"
 */

class GEM_EXTERN GEMglRotated : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglRotated, GemGLBase);

	public:
	  // Constructor
	  GEMglRotated (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglRotated ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	angle;		// VAR
	  virtual void	angleMess(t_float);	// FUN

	  GLdouble	x;		// VAR
	  virtual void	xMess(t_float);	// FUN

	  GLdouble	y;		// VAR
	  virtual void	yMess(t_float);	// FUN

	  GLdouble	z;		// VAR
	  virtual void	zMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 angleMessCallback (void*, t_floatarg);
	  static void	 xMessCallback (void*, t_floatarg);
	  static void	 yMessCallback (void*, t_floatarg);
	  static void	 zMessCallback (void*, t_floatarg);
};
#endif // for header file
