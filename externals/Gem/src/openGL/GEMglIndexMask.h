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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLINDEXMASK_H_
#define _INCLUDE__GEM_OPENGL_GEMGLINDEXMASK_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIndexMask
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIndexMask( GLuint mask)"
 */

class GEM_EXTERN GEMglIndexMask : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIndexMask, GemGLBase);

	public:
	  // Constructor
	  GEMglIndexMask (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglIndexMask ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLuint	mask;		// VAR
	  virtual void	maskMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 maskMessCallback (void*, t_floatarg);
};
#endif // for header file
