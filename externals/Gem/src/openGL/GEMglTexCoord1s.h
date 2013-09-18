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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_S_H_
#define _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_S_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglTexCoord1s
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glTexCoord1s( GLshort s)"
 */

class GEM_EXTERN GEMglTexCoord1s : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglTexCoord1s, GemGLBase);

	public:
	  // Constructor
	  GEMglTexCoord1s (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglTexCoord1s ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLshort	s;		// VAR
	  virtual void	sMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 sMessCallback (void*, t_floatarg);
};
#endif // for header file
