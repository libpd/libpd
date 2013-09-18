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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMAPGRID_D_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMAPGRID_D_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMapGrid2d
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glMapGrid2d( GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2)"
 */

class GEM_EXTERN GEMglMapGrid2d : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMapGrid2d, GemGLBase);

	public:
	  // Constructor
	  GEMglMapGrid2d (int, t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMapGrid2d ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLint	un;		// VAR
	  virtual void	unMess(t_float);	// FUN

	  GLdouble	u1;		// VAR
	  virtual void	u1Mess(t_float);	// FUN

	  GLdouble	u2;		// VAR
	  virtual void	u2Mess(t_float);	// FUN

	  GLint	vn;		// VAR
	  virtual void	vnMess(t_float);	// FUN

	  GLdouble	v1;		// VAR
	  virtual void	v1Mess(t_float);	// FUN

	  GLdouble	v2;		// VAR
	  virtual void	v2Mess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[6];

	// static member functions
	  static void	 unMessCallback (void*, t_floatarg);
	  static void	 u1MessCallback (void*, t_floatarg);
	  static void	 u2MessCallback (void*, t_floatarg);
	  static void	 vnMessCallback (void*, t_floatarg);
	  static void	 v1MessCallback (void*, t_floatarg);
	  static void	 v2MessCallback (void*, t_floatarg);
};
#endif // for header file
