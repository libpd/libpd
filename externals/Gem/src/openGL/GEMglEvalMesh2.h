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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLEVALMESH__H_
#define _INCLUDE__GEM_OPENGL_GEMGLEVALMESH__H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglEvalMesh2
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glEvalMesh2( GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2)"
 */

class GEM_EXTERN GEMglEvalMesh2 : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglEvalMesh2, GemGLBase);

	public:
	  // Constructor
	  GEMglEvalMesh2 (int, t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglEvalMesh2 ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	mode;		// VAR
	  virtual void	modeMess(t_float);	// FUN

	  GLint	i1;		// VAR
	  virtual void	i1Mess(t_float);	// FUN

	  GLint	i2;		// VAR
	  virtual void	i2Mess(t_float);	// FUN

	  GLint	j1;		// VAR
	  virtual void	j1Mess(t_float);	// FUN

	  GLint	j2;		// VAR
	  virtual void	j2Mess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[5];

	// static member functions
	  static void	 modeMessCallback (void*, t_floatarg);
	  static void	 i1MessCallback (void*, t_floatarg);
	  static void	 i2MessCallback (void*, t_floatarg);
	  static void	 j1MessCallback (void*, t_floatarg);
	  static void	 j2MessCallback (void*, t_floatarg);
};
#endif // for header file
