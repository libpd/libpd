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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPOPATTRIB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPOPATTRIB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglPopAttrib
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glPopAttrib()"
 */

class GEM_EXTERN GEMglPopAttrib : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglPopAttrib, GemGLBase);

	public:
	  // Constructor
	  GEMglPopAttrib ();	// CON

	protected:
	  // Destructor
	  virtual ~GEMglPopAttrib ();
	  // Do the rendering
	  virtual void	render (GemState *state);
};
#endif // for header file
