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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLBLENDEQUATION_H_
#define _INCLUDE__GEM_OPENGL_GEMGLBLENDEQUATION_H_

#include "Base/GemGLBase.h"

/*
  CLASS
  GEMglBlendEquation
  KEYWORDS
  openGL	0
  DESCRIPTION
  wrapper for the openGL-function
  "glBlendEquation( GLenum mode)"
*/

class GEM_EXTERN GEMglBlendEquation : public GemGLBase
{
  CPPEXTERN_HEADER(GEMglBlendEquation, GemGLBase);

    public:
  // Constructor
  GEMglBlendEquation (t_float);	// CON

 protected:
  // Destructor
  virtual ~GEMglBlendEquation ();
          // check extensions
          virtual bool isRunnable(void);

  // Do the rendering
  virtual void	render (GemState *state);

  // variables
  GLenum	mode;		// VAR
  virtual void	modeMess(t_float);	// FUN


 private:

  // we need some inlets
  t_inlet *m_inlet[1];

  // static member functions
  static void	 modeMessCallback (void*, t_floatarg);
};
#endif // for header file
