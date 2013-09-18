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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLDRAWARRAYS_H_
#define _INCLUDE__GEM_OPENGL_GEMGLDRAWARRAYS_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglDrawArrays
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glDrawArrays( GLenum mode, GLint first, GLsizei count)"
 */

class GEM_EXTERN GEMglDrawArrays : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglDrawArrays, GemGLBase);

	public:
	  // Constructor
	  GEMglDrawArrays (t_float, t_float, t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglDrawArrays ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	mode;		// VAR
	  virtual void	modeMess(t_float);	// FUN

	  GLint	first;		// VAR
	  virtual void	firstMess(t_float);	// FUN

	  GLsizei	count;		// VAR
	  virtual void	countMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[3];

	// static member functions
	  static void	 modeMessCallback (void*, t_floatarg);
	  static void	 firstMessCallback (void*, t_floatarg);
	  static void	 countMessCallback (void*, t_floatarg);
};
#endif // for header file
