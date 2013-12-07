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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLINDEXUB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLINDEXUB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIndexub
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIndexub( GLubyte c)"
 */

class GEM_EXTERN GEMglIndexub : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIndexub, GemGLBase);

	public:
	  // Constructor
	  GEMglIndexub (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglIndexub ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLubyte	c;		// VAR
	  virtual void	cMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 cMessCallback (void*, t_floatarg);
};
#endif // for header file
