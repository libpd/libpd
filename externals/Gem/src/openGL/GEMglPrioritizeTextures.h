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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPRIORITIZETEXTURES_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPRIORITIZETEXTURES_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglPrioritizeTextures
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glPrioritizeTextures( GLsizei n, GLuint *textures, GLclampf *priorities)"
 */

class GEM_EXTERN GEMglPrioritizeTextures : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglPrioritizeTextures, GemGLBase);

	public:
	  // Constructor
	  GEMglPrioritizeTextures (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglPrioritizeTextures ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLsizei	n;		// VAR
	  virtual void	nMess(t_float);	// FUN

	  GLuint*	textures;		// VAR
	  int t_len;
	  virtual void	texturesMess(int,t_atom*);	// FUN

	  GLclampf*	priorities;		// VAR
	  int p_len;
	  virtual void	prioritiesMess(int,t_atom*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 nMessCallback (void*, t_floatarg);
	  static void	 texturesMessCallback (void*, t_symbol*, int, t_atom*);
	  static void	 prioritiesMessCallback (void*, t_symbol*, int,t_atom*);
};
#endif // for header file
