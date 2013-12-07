 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2005 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  *  this file has been generated...
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLUPERSPECTIVE_H_
#define _INCLUDE__GEM_OPENGL_GEMGLUPERSPECTIVE_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMgluPerspective
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"gluPerspective( GLdouble fovy, GLdouble aspect, GLdouble near, GLdouble far)"
 */

class GEM_EXTERN GEMgluPerspective : public GemGLBase
{
	CPPEXTERN_HEADER(GEMgluPerspective, GemGLBase);

	public:
	  // Constructor
	  GEMgluPerspective (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMgluPerspective ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	fovy;		// VAR
	  virtual void	fovyMess(t_float);	// FUN

	  GLdouble	aspect;		// VAR
	  virtual void	aspectMess(t_float);	// FUN

	  GLdouble	m_near;		// VAR
	  virtual void	nearMess(t_float);	// FUN

	  GLdouble	m_far;		// VAR
	  virtual void	farMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 fovyMessCallback (void*, t_floatarg);
	  static void	 aspectMessCallback (void*, t_floatarg);
	  static void	 nearMessCallback (void*, t_floatarg);
	  static void	 farMessCallback (void*, t_floatarg);
};
#endif // for header file
