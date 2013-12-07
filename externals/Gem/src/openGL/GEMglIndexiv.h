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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLINDEXIV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLINDEXIV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIndexiv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIndexiv( GLint *c )"
 */

class GEM_EXTERN GEMglIndexiv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIndexiv, GemGLBase);

	public:
	  // Constructor
	  GEMglIndexiv (t_floatarg);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglIndexiv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLint c[1];		// VAR
	virtual void	cMess(t_float);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet;

	// static member functions
         static void    cMessCallback (void*, t_floatarg);
};
#endif // for header file
