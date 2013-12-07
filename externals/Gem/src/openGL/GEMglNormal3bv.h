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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLNORMAL_BV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLNORMAL_BV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglNormal3bv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glNormal3bv( GLbyte* v)"
 */

class GEM_EXTERN GEMglNormal3bv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglNormal3bv, GemGLBase);

	public:
	  // Constructor
	  GEMglNormal3bv (t_float, t_float, t_float);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglNormal3bv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLbyte	v[3];		// VAR
	virtual void	vMess(t_float, t_float, t_float);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet;

	// static member functions
         static void    vMessCallback (void*, t_floatarg, t_floatarg, t_floatarg);
};
#endif // for header file
