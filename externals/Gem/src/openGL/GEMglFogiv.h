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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLFOGIV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLFOGIV_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglFogiv
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glFogiv(GLenum pname, GLint *params)"
 */

#define FOG_ARRAY_LENGTH 4


class GEM_EXTERN GEMglFogiv : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglFogiv, GemGLBase);

	public:
	  // Constructor
	  GEMglFogiv (int,t_atom*);	// CON
	protected:
	  // Destructor
	  virtual ~GEMglFogiv ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variable
	GLenum          pname;
	virtual void	pnameMess(t_float);	// FUN

	GLint    	params[FOG_ARRAY_LENGTH];		// VAR
	virtual void	paramsMess(int, t_atom*);	// FUN

	private:

	// we need one inlet
	t_inlet *m_inlet[2];

	// static member functions
         static void    pnameMessCallback (void*, t_float);
         static void    paramsMessCallback (void*, t_symbol*, int, t_atom*);
};
#endif // for header file
