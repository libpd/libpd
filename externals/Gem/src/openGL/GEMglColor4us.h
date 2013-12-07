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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCOLOR_US_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCOLOR_US_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglColor4us
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glColor4us( GLushort red, GLushort green, GLushort blue, GLushort alpha)"
 */

class GEM_EXTERN GEMglColor4us : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglColor4us, GemGLBase);

	public:
	  // Constructor
	  GEMglColor4us (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglColor4us ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLushort	red;		// VAR
	  virtual void	redMess(t_float);	// FUN

	  GLushort	green;		// VAR
	  virtual void	greenMess(t_float);	// FUN

	  GLushort	blue;		// VAR
	  virtual void	blueMess(t_float);	// FUN

	  GLushort	alpha;		// VAR
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
