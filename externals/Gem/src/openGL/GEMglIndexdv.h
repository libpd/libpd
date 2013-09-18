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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLINDEXDV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLINDEXDV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIndexdv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIndexdv( GLdouble *c )"
 */

class GEM_EXTERN GEMglIndexdv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIndexdv, GemGLBase);

	public:
	  // Constructor
	  GEMglIndexdv (t_floatarg);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglIndexdv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLdouble c[1];		// VAR
	virtual void	cMess(t_float);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet;

	// static member functions
         static void    cMessCallback (void*, t_floatarg);
};
#endif // for header file
