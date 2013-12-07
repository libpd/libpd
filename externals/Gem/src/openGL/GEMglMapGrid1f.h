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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMAPGRID_F_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMAPGRID_F_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMapGrid1f
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glMapGrid1f( GLint un, GLfloat u1, GLfloat u2)"
 */

class GEM_EXTERN GEMglMapGrid1f : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMapGrid1f, GemGLBase);

	public:
	  // Constructor
	  GEMglMapGrid1f (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMapGrid1f ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	un;		// VAR
	  virtual void	unMess(t_float);	// FUN

	  GLfloat	u1;		// VAR
	  virtual void	u1Mess(t_float);	// FUN

	  GLfloat	u2;		// VAR
	  virtual void	u2Mess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 unMessCallback (void*, t_floatarg);
	  static void	 u1MessCallback (void*, t_floatarg);
	  static void	 u2MessCallback (void*, t_floatarg);
};
#endif // for header file
