 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMULTTRANSPOSEMATRIXD_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMULTTRANSPOSEMATRIXD_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMultTransposeMatrixd
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glMultTransposeMatrixd( GLdouble matrix)"
 */

class GEM_EXTERN GEMglMultTransposeMatrixd : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMultTransposeMatrixd, GemGLBase);

	public:
	  // Constructor
	  GEMglMultTransposeMatrixd (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMultTransposeMatrixd ();
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
	  static void	 matrixMessCallback (void*, t_symbol*,int,t_atom*);
};
#endif // for header file
