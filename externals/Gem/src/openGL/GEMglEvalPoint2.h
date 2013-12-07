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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLEVALPOINT__H_
#define _INCLUDE__GEM_OPENGL_GEMGLEVALPOINT__H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglEvalPoint2
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glEvalPoint2( GLint i, GLint j)"
 */

class GEM_EXTERN GEMglEvalPoint2 : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglEvalPoint2, GemGLBase);

	public:
	  // Constructor
	  GEMglEvalPoint2 (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglEvalPoint2 ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	i;		// VAR
	  virtual void	iMess(t_float);	// FUN

	  GLint	j;		// VAR
	  virtual void	jMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 iMessCallback (void*, t_floatarg);
	  static void	 jMessCallback (void*, t_floatarg);
};
#endif // for header file
