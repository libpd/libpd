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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLRASTERPOS_SV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLRASTERPOS_SV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglRasterPos2sv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glRasterPos2sv( GLshort* v)"
 */

class GEM_EXTERN GEMglRasterPos2sv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglRasterPos2sv, GemGLBase);

	public:
	  // Constructor
	  GEMglRasterPos2sv (t_float, t_float);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglRasterPos2sv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLshort	v[2];		// VAR
	virtual void	vMess(t_float, t_float);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet;

	// static member functions
         static void    vMessCallback (void*, t_floatarg, t_floatarg);
};
#endif // for header file
