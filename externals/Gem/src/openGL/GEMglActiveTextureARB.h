 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2004-2005 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLACTIVETEXTUREARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLACTIVETEXTUREARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglActiveTextureARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glActiveTextureARB( GLenum target )"
 */

class GEM_EXTERN GEMglActiveTextureARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglActiveTextureARB, GemGLBase);

	public:
	  // Constructor
	  GEMglActiveTextureARB (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglActiveTextureARB ();
          // check extensions
          virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	texUnit;		// VAR
	  virtual void	texUnitMess(t_float);	// FUN

	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 texUnitMessCallback (void*, t_floatarg);
};
#endif // for header file
