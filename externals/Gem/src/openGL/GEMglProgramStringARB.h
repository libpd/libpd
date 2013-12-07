 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  *
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLPROGRAMSTRINGARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLPROGRAMSTRINGARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglProgramStringARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glProgramStringARB( GLenum target, GLenum format, GLsizei len, GLvoid *string)"
 */

class GEM_EXTERN GEMglProgramStringARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglProgramStringARB, GemGLBase);

	public:
	  // Constructor
          GEMglProgramStringARB (int, t_atom*);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglProgramStringARB ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum		target;		// VAR
	  virtual void	targetMess(t_float);	// FUN

	  GLenum		format;		// VAR
	  virtual void	formatMess(t_float);	// FUN

	  GLsizei		len;		// VAR
	  virtual void	lenMess(t_float);	// FUN

	  GLvoid	*string;		// VAR
	  virtual void	stringMess(t_symbol*);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[4];

	// static member functions
	  static void	 targetMessCallback (void*, t_floatarg);
	  static void	 formatMessCallback (void*, t_floatarg);
	  static void	 lenMessCallback (void*, t_floatarg);
	  static void	 stringMessCallback (void*, t_symbol*);
};
#endif // for header file
