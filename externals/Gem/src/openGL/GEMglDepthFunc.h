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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLDEPTHFUNC_H_
#define _INCLUDE__GEM_OPENGL_GEMGLDEPTHFUNC_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglDepthFunc
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glDepthFunc( GLenum func)"
 */

class GEM_EXTERN GEMglDepthFunc : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglDepthFunc, GemGLBase);

	public:
	  // Constructor
	  GEMglDepthFunc (int, t_atom*); // CON

	protected:
	  // Destructor
	  virtual ~GEMglDepthFunc ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	func;		// VAR
	  virtual void	funcMess(t_atom);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 funcMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
