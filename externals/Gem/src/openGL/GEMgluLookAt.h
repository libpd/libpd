/* ------------------------------------------------------------------
 * GEM - Graphics Environment for Multimedia
 *
 *  Copyright (c) 2008 zmoelnig@iem.at
 *  For information on usage and redistribution, and for a DISCLAIMER
 *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
 *
 *  this file has been generated...
 * ------------------------------------------------------------------
 */

#ifndef _INCLUDE__GEM_OPENGL_GEMGLULOOKAT_H_
#define _INCLUDE__GEM_OPENGL_GEMGLULOOKAT_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMgluLookAt
 KEYWORDS
	openGL
 DESCRIPTION
	wrapper for the openGL-function
	"void gluLookAt (GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ);"
 */

class GEM_EXTERN GEMgluLookAt : public GemGLBase
{
	CPPEXTERN_HEADER(GEMgluLookAt, GemGLBase);

	public:
	  // Constructor
  GEMgluLookAt (int, t_atom*);

	protected:
	  // Destructor
	  virtual ~GEMgluLookAt ();
	  // Do the rendering
	  virtual void	render (GemState *state);

    // variables
	  GLdouble m_eyeX; // VAR
	  virtual void eyeXMess( t_float ); // VAR

	  GLdouble m_eyeY; // VAR
	  virtual void eyeYMess( t_float ); // VAR

	  GLdouble m_eyeZ; // VAR
	  virtual void eyeZMess( t_float ); // VAR

	  GLdouble m_centerX; // VAR
	  virtual void centerXMess( t_float ); // VAR

	  GLdouble m_centerY; // VAR
	  virtual void centerYMess( t_float ); // VAR

	  GLdouble m_centerZ; // VAR
	  virtual void centerZMess( t_float ); // VAR

	  GLdouble m_upX; // VAR
	  virtual void upXMess( t_float ); // VAR

	  GLdouble m_upY; // VAR
	  virtual void upYMess( t_float ); // VAR

	  GLdouble m_upZ; // VAR
	  virtual void upZMess( t_float ); // VAR


	private:

    // we need some inlets
	  t_inlet *m_inlet[9];

    // static member functions
	  static void eyeXMessCallback(void*, t_floatarg );
	  static void eyeYMessCallback(void*, t_floatarg );
	  static void eyeZMessCallback(void*, t_floatarg );
	  static void centerXMessCallback(void*, t_floatarg );
	  static void centerYMessCallback(void*, t_floatarg );
	  static void centerZMessCallback(void*, t_floatarg );
	  static void upXMessCallback(void*, t_floatarg );
	  static void upYMessCallback(void*, t_floatarg );
	  static void upZMessCallback(void*, t_floatarg );
};
#endif /* for header file */
