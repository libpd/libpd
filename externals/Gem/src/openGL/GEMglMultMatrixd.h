 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMULTMATRIXD_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMULTMATRIXD_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMultMatrixd
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glMultMatrixd( GLfloat matrix)"
 */

class GEM_EXTERN GEMglMultMatrixd : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMultMatrixd, GemGLBase);

	public:
	  // Constructor
	  GEMglMultMatrixd (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMultMatrixd ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	m_matrix[16];		// VAR
	  virtual void	matrixMess(int argc, t_atom*argv);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet;

	// static member functions
	  static void	 matrixMessCallback (void*, t_symbol*, int, t_atom*);
};
#endif // for header file
