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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLFLUSH_H_
#define _INCLUDE__GEM_OPENGL_GEMGLFLUSH_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglFlush
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glFlush()"
 */

class GEM_EXTERN GEMglFlush : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglFlush, GemGLBase);

	public:
	  // Constructor
	  GEMglFlush ();	// CON

	protected:
	  // Destructor
	  virtual ~GEMglFlush ();
	  // Do the rendering
	  virtual void	render (GemState *state);

};
#endif // for header file
