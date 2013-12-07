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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMAP_D_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMAP_D_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMap1d
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glMap1d( GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, GLdouble *points)"
 */

class GEM_EXTERN GEMglMap1d : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMap1d, GemGLBase);

	public:
	  // Constructor
	  GEMglMap1d (int,t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMap1d ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLdouble	u1;		// VAR
	  virtual void	u1Mess(t_float);	// FUN

	  GLdouble	u2;		// VAR
	  virtual void	u2Mess(t_float);	// FUN

	  GLint	stride;		// VAR
	  virtual void	strideMess(t_float);	// FUN

	  GLint	order;		// VAR
	  virtual void	orderMess(t_float);	// FUN

	  int len;
	  GLdouble*	points;		// VAR
	  virtual void	pointsMess(int,t_atom*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[6];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 u1MessCallback (void*, t_floatarg);
	  static void	 u2MessCallback (void*, t_floatarg);
	  static void	 strideMessCallback (void*, t_floatarg);
	  static void	 orderMessCallback (void*, t_floatarg);
	  static void	 pointsMessCallback (void*, t_symbol*, int,t_atom*);
};
#endif // for header file
