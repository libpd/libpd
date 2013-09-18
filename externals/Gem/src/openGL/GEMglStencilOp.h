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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLSTENCILOP_H_
#define _INCLUDE__GEM_OPENGL_GEMGLSTENCILOP_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglStencilOp
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glStencilOp( GLenum fail, GLenum zfail, GLenum zpass)"
 */

class GEM_EXTERN GEMglStencilOp : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglStencilOp, GemGLBase);

	public:
	  // Constructor
	  GEMglStencilOp (int, t_atom*); // CON

	protected:
	  // Destructor
	  virtual ~GEMglStencilOp ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	fail;		// VAR
	  virtual void	failMess(t_atom);	// FUN

	  GLenum	zfail;		// VAR
	  virtual void	zfailMess(t_atom);	// FUN

	  GLenum	zpass;		// VAR
	  virtual void	zpassMess(t_atom);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 failMessCallback (void*,t_symbol*,int,t_atom*);
	  static void	 zfailMessCallback (void*,t_symbol*,int,t_atom*);
	  static void	 zpassMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
