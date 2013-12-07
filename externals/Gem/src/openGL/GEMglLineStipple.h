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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLLINESTIPPLE_H_
#define _INCLUDE__GEM_OPENGL_GEMGLLINESTIPPLE_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglLineStipple
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glLineStipple( GLint factor, GLushort pattern)"
 */

class GEM_EXTERN GEMglLineStipple : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglLineStipple, GemGLBase);

	public:
	  // Constructor
	  GEMglLineStipple (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglLineStipple ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	factor;		// VAR
	  virtual void	factorMess(t_float);	// FUN

	  GLushort	pattern;		// VAR
	  virtual void	patternMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 factorMessCallback (void*, t_floatarg);
	  static void	 patternMessCallback (void*, t_floatarg);
};
#endif // for header file
