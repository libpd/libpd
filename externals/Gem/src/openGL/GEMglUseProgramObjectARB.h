 /* ------------------------------------------------------------------
  * GEM - Graphics Environment for Multimedia
  *
  *  Copyright (c) 2005 tigital@mac.com
  *  For information on usage and redistribution, and for a DISCLAIMER
  *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
  * ------------------------------------------------------------------
  */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLUSEPROGRAMOBJECTARB_H_
#define _INCLUDE__GEM_OPENGL_GEMGLUSEPROGRAMOBJECTARB_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglUseProgramObjectARB
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glUseProgramObjectARB( GLenum program )"
 */

class GEM_EXTERN GEMglUseProgramObjectARB : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglUseProgramObjectARB, GemGLBase);

	public:
	  // Constructor
	  GEMglUseProgramObjectARB ();	// CON

	protected:
	  // Destructor
	  virtual ~GEMglUseProgramObjectARB ();
	  
    // check extensions
    virtual bool isRunnable(void);

	  // Do the rendering
	  virtual void	render (GemState *state);
	  
	  // clean up the postrendering
	  virtual void	postrender (GemState *state);

    // variables
	  GLhandleARB	m_program;		// VAR
	  virtual void	programMess(int);

	private:

	// we need some inlets
	  t_inlet *m_inlet;

	// static member functions
	  static void	 programMessCallback (void*, t_floatarg);
};
#endif // for header file
