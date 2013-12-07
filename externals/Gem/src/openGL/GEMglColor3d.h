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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCOLOR_D_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCOLOR_D_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglColor3d
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glColor3d( GLdouble red, GLdouble green, GLdouble blue)"
 */

class GEM_EXTERN GEMglColor3d : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglColor3d, GemGLBase);

	public:
	  // Constructor
	  GEMglColor3d (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglColor3d ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	red;		// VAR
	  virtual void	redMess(t_float);	// FUN

	  GLdouble	green;		// VAR
	  virtual void	greenMess(t_float);	// FUN

	  GLdouble	blue;		// VAR
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
