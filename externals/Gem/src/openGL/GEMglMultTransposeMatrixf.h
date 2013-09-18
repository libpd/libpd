 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMULTTRANSPOSEMATRIXF_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMULTTRANSPOSEMATRIXF_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMultTransposeMatrixf
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glMultTransposeMatrixf( GLfloat matrix)"
 */

class GEM_EXTERN GEMglMultTransposeMatrixf : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMultTransposeMatrixf, GemGLBase);

	public:
	  // Constructor
	  GEMglMultTransposeMatrixf (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMultTransposeMatrixf ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	m_matrix[16];		// VAR
	  virtual void	matrixMess(int argc, t_atom*argv);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet;

	// static member functions
	  static void	 matrixMessCallback (void*, t_symbol*,int,t_atom*);
};
#endif // for header file
