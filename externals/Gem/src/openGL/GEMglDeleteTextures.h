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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLDELETETEXTURES_H_
#define _INCLUDE__GEM_OPENGL_GEMGLDELETETEXTURES_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglDeleteTextures
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glDeleteTextures( GLsizei n, GLuint *textures)"
 */

class GEM_EXTERN GEMglDeleteTextures : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglDeleteTextures, GemGLBase);

	public:
	  // Constructor
	  GEMglDeleteTextures (int,t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglDeleteTextures ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLsizei	n;		// VAR
	  GLuint*	textures;		// VAR
	  virtual void	texturesMess(int,t_atom*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet;

	// static member functions
	  static void	 texturesMessCallback (void*, t_symbol*, int,t_atom*);
};
#endif // for header file
