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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLFOGFV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLFOGFV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglFogfv
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glFogfv( GLenum pname, GLfloat *params)"
 */

#define FOG_ARRAY_LENGTH 4


class GEM_EXTERN GEMglFogfv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglFogfv, GemGLBase);

	public:
	  // Constructor
	  GEMglFogfv (int, t_atom*);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglFogfv ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLenum pname;
	virtual void	pnameMess(t_float);	// FUN

	GLfloat 	params[FOG_ARRAY_LENGTH];		// VAR
	virtual void	paramsMess(int, t_atom*);	// FUN

	private:

	// we need one inlet
	  t_inlet *m_inlet[2];

	// static member functions
         static void    pnameMessCallback (void*, t_float);
         static void    paramsMessCallback (void*, t_symbol*, int, t_atom*);

};
#endif // for header file
