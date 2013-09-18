 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLBINDPROGRAMARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLBINDPROGRAMARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglBindProgramARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glBindProgramARB( GLenum target, GLuint program)"
 */

class GEM_EXTERN GEMglBindProgramARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglBindProgramARB, GemGLBase);

	public:
	  // Constructor
	  GEMglBindProgramARB (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglBindProgramARB ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLuint	program;		// VAR
	  virtual void	programMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 programMessCallback (void*, t_floatarg);
};
#endif // for header file
