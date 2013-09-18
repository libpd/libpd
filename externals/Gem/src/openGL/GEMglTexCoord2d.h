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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_D_H_
#define _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_D_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglTexCoord2d
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glTexCoord2d( GLdouble s, GLdouble t)"
 */

class GEM_EXTERN GEMglTexCoord2d : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglTexCoord2d, GemGLBase);

	public:
	  // Constructor
	  GEMglTexCoord2d (t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglTexCoord2d ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLdouble	s;		// VAR
	  virtual void	sMess(t_float);	// FUN

	  GLdouble	t;		// VAR
	  virtual void	tMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[2];

	// static member functions
	  static void	 sMessCallback (void*, t_floatarg);
	  static void	 tMessCallback (void*, t_floatarg);
};
#endif // for header file
