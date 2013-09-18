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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLACCUM_H_
#define _INCLUDE__GEM_OPENGL_GEMGLACCUM_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglAccum
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glAccum( GLenum op, GLfloat value)"
 */

class GEM_EXTERN GEMglAccum : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglAccum, GemGLBase);

	public:
	  // Constructor
	  GEMglAccum (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglAccum ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	op;		// VAR
	  virtual void	opMess(t_float);	// FUN

	  GLfloat	value;		// VAR
	  virtual void	valueMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 opMessCallback (void*, t_floatarg);
	  static void	 valueMessCallback (void*, t_floatarg);
};
#endif // for header file
