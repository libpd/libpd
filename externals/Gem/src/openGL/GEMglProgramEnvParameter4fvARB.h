 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPROGRAMENVPARAMETER_FVARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPROGRAMENVPARAMETER_FVARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglProgramEnvParameter4fvARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glProgramEnvParameter4fvARB( GLenum target, GLuint index, GLfloat *params)"
		target = GL_VERTEX_PROGRAM_ARB or GL_FRAGMENT_PROGRAM_ARB
		index = id number of program
		params = vector of 4 floats
 */

class GEM_EXTERN GEMglProgramEnvParameter4fvARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglProgramEnvParameter4fvARB, GemGLBase);

	public:
	  // Constructor
	  GEMglProgramEnvParameter4fvARB (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglProgramEnvParameter4fvARB ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLuint	index;		// VAR
	  virtual void	indexMess(t_float);	// FUN

	  GLfloat	params[4];		// VAR
	  virtual void	paramsMess(int argc, t_atom* argv);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 indexMessCallback (void*, t_floatarg);
	  static void	 paramsMessCallback (void*, t_symbol*, int, t_atom*);
};
#endif // for header file
