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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLNORMAL_I_H_
#define _INCLUDE__GEM_OPENGL_GEMGLNORMAL_I_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglNormal3i
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glNormal3i( GLint nx, GLint ny, GLint nz)"
 */

class GEM_EXTERN GEMglNormal3i : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglNormal3i, GemGLBase);

	public:
	  // Constructor
	  GEMglNormal3i (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglNormal3i ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	nx;		// VAR
	  virtual void	nxMess(t_float);	// FUN

	  GLint	ny;		// VAR
	  virtual void	nyMess(t_float);	// FUN

	  GLint	nz;		// VAR
	  virtual void	nzMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 nxMessCallback (void*, t_floatarg);
	  static void	 nyMessCallback (void*, t_floatarg);
	  static void	 nzMessCallback (void*, t_floatarg);
};
#endif // for header file
