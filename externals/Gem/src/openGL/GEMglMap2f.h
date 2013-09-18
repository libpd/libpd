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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLMAP_F_H_
#define _INCLUDE__GEM_OPENGL_GEMGLMAP_F_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglMap2f
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glMap2f( GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat *points)"
 */

class GEM_EXTERN GEMglMap2f : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglMap2f, GemGLBase);

	public:
	  // Constructor
	  GEMglMap2f (int,t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglMap2f ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLfloat	u1;		// VAR
	  virtual void	u1Mess(t_float);	// FUN

	  GLfloat	u2;		// VAR
	  virtual void	u2Mess(t_float);	// FUN

	  GLint	ustride;		// VAR
	  virtual void	ustrideMess(t_float);	// FUN

	  GLint	uorder;		// VAR
	  virtual void	uorderMess(t_float);	// FUN

	  GLfloat	v1;		// VAR
	  virtual void	v1Mess(t_float);	// FUN

	  GLfloat	v2;		// VAR
	  virtual void	v2Mess(t_float);	// FUN

	  GLint	vstride;		// VAR
	  virtual void	vstrideMess(t_float);	// FUN

	  GLint	vorder;		// VAR
	  virtual void	vorderMess(t_float);	// FUN

	  GLfloat*	points;		// VAR
	  int len;
	  virtual void	pointsMess(int,t_atom*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[10];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 u1MessCallback (void*, t_floatarg);
	  static void	 u2MessCallback (void*, t_floatarg);
	  static void	 ustrideMessCallback (void*, t_floatarg);
	  static void	 uorderMessCallback (void*, t_floatarg);
	  static void	 v1MessCallback (void*, t_floatarg);
	  static void	 v2MessCallback (void*, t_floatarg);
	  static void	 vstrideMessCallback (void*, t_floatarg);
	  static void	 vorderMessCallback (void*, t_floatarg);
	  static void	 pointsMessCallback (void*, t_symbol*, int,t_atom*);
};
#endif // for header file
