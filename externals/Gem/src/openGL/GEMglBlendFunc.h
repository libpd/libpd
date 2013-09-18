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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLBLENDFUNC_H_
#define _INCLUDE__GEM_OPENGL_GEMGLBLENDFUNC_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglBlendFunc
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glBlendFunc( GLenum sfactor, GLenum dfactor)"
 */

class GEM_EXTERN GEMglBlendFunc : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglBlendFunc, GemGLBase);

	public:
	  // Constructor
	  GEMglBlendFunc (int, t_atom*); // CON

	protected:
	  // Destructor
	  virtual ~GEMglBlendFunc ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	sfactor;		// VAR
	  virtual void	sfactorMess(t_atom);	// FUN

	  GLenum	dfactor;		// VAR
	  virtual void	dfactorMess(t_atom);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 sfactorMessCallback (void*,t_symbol*,int,t_atom*);
	  static void	 dfactorMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
