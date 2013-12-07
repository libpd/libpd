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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLGETINTEGERV_H_
#define _INCLUDE__GEM_OPENGL_GEMGLGETINTEGERV_H_

#include "Base/GemGLBase.h"

/*
  CLASS
  GEMglGetIntegerv
  KEYWORDS
  openGL	0
  DESCRIPTION
  wrapper for the openGL-function
  "glGetIntegerv( GLenum pname, GLint *params)"
*/

class GEM_EXTERN GEMglGetIntegerv : public GemGLBase
{
  CPPEXTERN_HEADER(GEMglGetIntegerv, GemGLBase);

    public:
  // Constructor
  GEMglGetIntegerv (int,t_atom*);	// CON
 protected:
  // Destructor
  virtual ~GEMglGetIntegerv ();
  // check extensions
  virtual bool isRunnable(void);

  // Do the rendering
  virtual void	render (GemState *state);
  // variable
  GLenum pname;
  virtual void	pnameMess(t_atom);	// FUN

  t_atom m_alist[16];

 private:
  // we need one inlet
  t_inlet *m_inlet;
  // The outlet
  t_outlet    	*m_outlet;

  // static member functions
  static void    pnameMessCallback (void*,t_symbol*,int,t_atom*);
};
#endif // for header file
