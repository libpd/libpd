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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLGETSTRING_H_
#define _INCLUDE__GEM_OPENGL_GEMGLGETSTRING_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglGetString
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glGetString( glGetString  GLenum name )"
 */

class GEM_EXTERN GEMglGetString : public GemGLBase
{
  CPPEXTERN_HEADER(GEMglGetString, GemGLBase);

    public:
  // Constructor
  GEMglGetString (t_floatarg);	// CON
  
 protected:
  // Destructor
  virtual ~GEMglGetString ();
  // Do the rendering
  virtual void	render (GemState *state);
  
  // variables
  GLenum name;		// VAR
  virtual void	nameMess(t_atom);	// FUN
  
  // we need some inl/outets
  t_inlet *m_inlet;
  t_outlet*m_outlet;

 private:

  // static member functions
  static void	 nameMessCallback (void*, t_symbol*,int,t_atom*);
};
#endif // for header file
