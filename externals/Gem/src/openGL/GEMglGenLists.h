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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLGENLISTS_H_
#define _INCLUDE__GEM_OPENGL_GEMGLGENLISTS_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglGenLists
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glGenLists( GLsizei range )"
 */

class GEM_EXTERN GEMglGenLists : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglGenLists, GemGLBase);

	public:
	  // Constructor
	GEMglGenLists (t_floatarg);	// CON

	protected:
	  // Destructor
	virtual ~GEMglGenLists ();
	// Do the rendering
	virtual void	render (GemState *state);

	// variables
	GLsizei	range;		// VAR
	virtual void	rangeMess(t_float);	// FUN

	private:

	// we need some in/outlets
	t_inlet *m_inlet;
	t_outlet*m_outlet;

	// static member functions
	  static void	 rangeMessCallback (void*, t_floatarg);
};
#endif // for header file
