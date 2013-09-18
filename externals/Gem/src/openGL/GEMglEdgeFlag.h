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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLEDGEFLAG_H_
#define _INCLUDE__GEM_OPENGL_GEMGLEDGEFLAG_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglEdgeFlag
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glEdgeFlag( GLboolean flag)"
 */

class GEM_EXTERN GEMglEdgeFlag : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglEdgeFlag, GemGLBase);

	public:
	  // Constructor
	  GEMglEdgeFlag (t_float);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglEdgeFlag ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLboolean	flag;		// VAR
	  virtual void	flagMess(t_float);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 flagMessCallback (void*, t_floatarg);
};
#endif // for header file
