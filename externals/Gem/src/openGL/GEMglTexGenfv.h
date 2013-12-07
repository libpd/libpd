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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLTEXGENFV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLTEXGENFV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglTexGenfv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glTexGenfv( GLenum coord, GLenum pname, GLfloat[4] params)"
 */

class GEM_EXTERN GEMglTexGenfv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglTexGenfv, GemGLBase);

	public:
	  // Constructor
	  GEMglTexGenfv (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglTexGenfv ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	coord;		// VAR
	  virtual void	coordMess(t_float);	// FUN

	  GLenum	pname;		// VAR
	  virtual void	pnameMess(t_float);	// FUN

	  GLfloat	params[4];		// VAR
	  virtual void	paramsMess(int argc, t_atom* argv);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 coordMessCallback (void*, t_floatarg);
	  static void	 pnameMessCallback (void*, t_floatarg);
	  static void	 paramsMessCallback (void*, t_symbol*, int, t_atom*);
};
#endif // for header file
