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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCLEARDEPTH_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCLEARDEPTH_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglClearDepth
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glClearDepth( GLclampd depth)"
 */

class GEM_EXTERN GEMglClearDepth : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglClearDepth, GemGLBase);

	public:
	  // Constructor
	  GEMglClearDepth (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglClearDepth ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLclampd	depth;		// VAR
	  virtual void	depthMess(float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 depthMessCallback (void*, float);
};
#endif // for header file
