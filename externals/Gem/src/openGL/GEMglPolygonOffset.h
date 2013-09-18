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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPOLYGONOFFSET_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPOLYGONOFFSET_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglPolygonOffset
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glPolygonOffset( GLfloat factor, GLfloat units)"
 */

class GEM_EXTERN GEMglPolygonOffset : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglPolygonOffset, GemGLBase);

	public:
	  // Constructor
	  GEMglPolygonOffset (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglPolygonOffset ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	factor;		// VAR
	  virtual void	factorMess(t_float);	// FUN

	  GLfloat	units;		// VAR
	  virtual void	unitsMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 factorMessCallback (void*, t_floatarg);
	  static void	 unitsMessCallback (void*, t_floatarg);
};
#endif // for header file
