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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCOLOR_UBV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCOLOR_UBV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglColor4ubv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glColor4ubv( GLubyte* v)"
 */

class GEM_EXTERN GEMglColor4ubv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglColor4ubv, GemGLBase);

	public:
	  // Constructor
	  GEMglColor4ubv (t_float, t_float, t_float, t_float);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglColor4ubv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLubyte	v[4];		// VAR
	virtual void	vMess(t_float, t_float, t_float, t_float);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet;

	// static member functions
         static void    vMessCallback (void*, t_floatarg, t_floatarg, t_floatarg, t_floatarg);
};
#endif // for header file
