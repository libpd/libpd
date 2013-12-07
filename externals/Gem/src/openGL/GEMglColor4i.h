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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCOLOR_I_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCOLOR_I_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglColor4i
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glColor4i( GLint red, GLint green, GLint blue, GLint alpha)"
 */

class GEM_EXTERN GEMglColor4i : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglColor4i, GemGLBase);

	public:
	  // Constructor
	  GEMglColor4i (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglColor4i ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	red;		// VAR
	  virtual void	redMess(t_float);	// FUN

	  GLint	green;		// VAR
	  virtual void	greenMess(t_float);	// FUN

	  GLint	blue;		// VAR
	  virtual void	blueMess(t_float);	// FUN

	  GLint	alpha;		// VAR
	  virtual void	alphaMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 redMessCallback (void*, t_floatarg);
	  static void	 greenMessCallback (void*, t_floatarg);
	  static void	 blueMessCallback (void*, t_floatarg);
	  static void	 alphaMessCallback (void*, t_floatarg);
};
#endif // for header file
