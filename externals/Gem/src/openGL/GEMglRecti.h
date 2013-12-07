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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLRECTI_H_
#define _INCLUDE__GEM_OPENGL_GEMGLRECTI_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglRecti
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glRecti( GLint x1, GLint y1, GLint x2, GLint y2)"
 */

class GEM_EXTERN GEMglRecti : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglRecti, GemGLBase);

	public:
	  // Constructor
	  GEMglRecti (t_float, t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglRecti ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	x1;		// VAR
	  virtual void	x1Mess(t_float);	// FUN

	  GLint	y1;		// VAR
	  virtual void	y1Mess(t_float);	// FUN

	  GLint	x2;		// VAR
	  virtual void	x2Mess(t_float);	// FUN

	  GLint	y2;		// VAR
	  virtual void	y2Mess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 x1MessCallback (void*, t_floatarg);
	  static void	 y1MessCallback (void*, t_floatarg);
	  static void	 x2MessCallback (void*, t_floatarg);
	  static void	 y2MessCallback (void*, t_floatarg);
};
#endif // for header file
