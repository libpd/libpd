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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_F_H_
#define _INCLUDE__GEM_OPENGL_GEMGLTEXCOORD_F_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglTexCoord4f
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glTexCoord4f( GLfloat s, GLfloat t, GLfloat r, GLfloat q)"
 */

class GEM_EXTERN GEMglTexCoord4f : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglTexCoord4f, GemGLBase);

	public:
	  // Constructor
	  GEMglTexCoord4f (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglTexCoord4f ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLfloat	s;		// VAR
	  virtual void	sMess(t_float);	// FUN

	  GLfloat	t;		// VAR
	  virtual void	tMess(t_float);	// FUN

	  GLfloat	r;		// VAR
	  virtual void	rMess(t_float);	// FUN

	  GLfloat	q;		// VAR
	  virtual void	qMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 sMessCallback (void*, t_floatarg);
	  static void	 tMessCallback (void*, t_floatarg);
	  static void	 rMessCallback (void*, t_floatarg);
	  static void	 qMessCallback (void*, t_floatarg);
};
#endif // for header file
