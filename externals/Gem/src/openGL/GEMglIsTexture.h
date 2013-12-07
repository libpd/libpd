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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLISTEXTURE_H_
#define _INCLUDE__GEM_OPENGL_GEMGLISTEXTURE_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIsTexture
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIsTexture( GLuint texture )"
 */

class GEM_EXTERN GEMglIsTexture : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIsTexture, GemGLBase);

	public:
	  // Constructor
	  GEMglIsTexture (t_floatarg);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglIsTexture ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLuint texture;		// VAR
	  virtual void	textureMess(t_float);	// FUN

	private:

	// we need some inlets
	  t_inlet *m_inlet;
	  t_outlet*m_outlet;

	// static member functions
	  static void	 textureMessCallback (void*, t_floatarg);
};
#endif // for header file
