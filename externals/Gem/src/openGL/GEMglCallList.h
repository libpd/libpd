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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLCALLLIST_H_
#define _INCLUDE__GEM_OPENGL_GEMGLCALLLIST_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglCallList
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glCallList( GLuint list )"
 */

class GEM_EXTERN GEMglCallList : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglCallList, GemGLBase);

	public:
	  // Constructor
	  GEMglCallList (t_floatarg);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglCallList ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLuint list;		// VAR
	  virtual void	listMess(t_float);	// FUN

	private:

	// we need some inlets
	  t_inlet *m_inlet;

	// static member functions
	  static void	 listMessCallback (void*, t_floatarg);
};
#endif // for header file
