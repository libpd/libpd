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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCOLOR_UB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCOLOR_UB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglColor3ub
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glColor3ub( GLubyte red, GLubyte green, GLubyte blue)"
 */

class GEM_EXTERN GEMglColor3ub : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglColor3ub, GemGLBase);

	public:
	  // Constructor
	  GEMglColor3ub (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglColor3ub ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLubyte	red;		// VAR
	  virtual void	redMess(t_float);	// FUN

	  GLubyte	green;		// VAR
	  virtual void	greenMess(t_float);	// FUN

	  GLubyte	blue;		// VAR
	  virtual void	blueMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 redMessCallback (void*, t_floatarg);
	  static void	 greenMessCallback (void*, t_floatarg);
	  static void	 blueMessCallback (void*, t_floatarg);
};
#endif // for header file
