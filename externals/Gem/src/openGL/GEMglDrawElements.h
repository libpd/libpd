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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLDRAWELEMENTS_H_
#define _INCLUDE__GEM_OPENGL_GEMGLDRAWELEMENTS_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglDrawElements
 KEYWORDS
	openGL	1
 DESCRIPTION
	wrapper for the openGL-function
	"glDrawElements( GLenum mode, GLsizei count, GLenum type, GLvoid *indices)"
 */

class GEM_EXTERN GEMglDrawElements : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglDrawElements, GemGLBase);

	public:
	  // Constructor
	  GEMglDrawElements (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglDrawElements ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	mode;		// VAR
	  virtual void	modeMess(t_float);	// FUN

	  GLsizei	count;		// VAR
	  virtual void	countMess(t_float);	// FUN

	  GLenum	type;		// VAR
	  virtual void	typeMess(t_float);	// FUN

	  GLuint	*indices_ui;		// VAR
	  GLubyte	*indices_ub;		// VAR
	  GLushort	*indices_us;		// VAR
	  int           len;

	  virtual void	indicesMess(int, t_atom*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 modeMessCallback (void*, t_floatarg);
	  static void	 countMessCallback (void*, t_floatarg);
	  static void	 typeMessCallback (void*, t_floatarg);
	  static void	 indicesMessCallback (void*, t_symbol*, int, t_atom*);
};
#endif // for header file
