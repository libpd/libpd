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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLLOGICOP_H_
#define _INCLUDE__GEM_OPENGL_GEMGLLOGICOP_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglLogicOp
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL-function
	"glLogicOp( GLenum opcode)"
 */

class GEM_EXTERN GEMglLogicOp : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglLogicOp, GemGLBase);

	public:
	  // Constructor
  GEMglLogicOp (int, t_atom*); // CON

	protected:
	  // Destructor
	  virtual ~GEMglLogicOp ();
	  // Do the rendering
	  virtual void	render (GemState *state);

	// variables
	  GLenum	opcode;		// VAR
	  virtual void	opcodeMess(t_atom);	// FUN


	private:

	// we need some inlets
	  t_inlet *m_inlet[1];

	// static member functions
	  static void	 opcodeMessCallback (void*, t_symbol*,int,t_atom*);
};
#endif // for header file
