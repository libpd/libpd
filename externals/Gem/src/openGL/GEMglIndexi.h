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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLINDEXI_H_
#define _INCLUDE__GEM_OPENGL_GEMGLINDEXI_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglIndexi
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glIndexi( GLint c)"
 */

class GEM_EXTERN GEMglIndexi : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglIndexi, GemGLBase);

	public:
	  // Constructor
	  GEMglIndexi (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglIndexi ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	c;		// VAR
	  virtual void	cMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 cMessCallback (void*, t_floatarg);
};
#endif // for header file
