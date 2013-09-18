 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPROGRAMLOCALPARAMETER_FVARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPROGRAMLOCALPARAMETER_FVARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglProgramLocalParameter4fvARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glProgramLocalParameter4fvARB( GLenum target, GLuint index, GLfloat *params)"
 */

class GEM_EXTERN GEMglProgramLocalParameter4fvARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglProgramLocalParameter4fvARB, GemGLBase);

	public:
	  // Constructor
	  GEMglProgramLocalParameter4fvARB (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglProgramLocalParameter4fvARB ();
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
	  static void	 paramsMessCallback (void*, t_symbol*,int, t_atom*);
};
#endif // for header file
