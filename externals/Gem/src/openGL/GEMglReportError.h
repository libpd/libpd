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

#ifndef _INCLUDE__GEM_OPENGL_GEMGLREPORTERROR_H_
#define _INCLUDE__GEM_OPENGL_GEMGLREPORTERROR_H_

#include "Base/GemGLBase.h"

/*
 CLASS
	GEMglReportError
 KEYWORDS
	openGL	0
 DESCRIPTION
	wrapper for the openGL defines
 */

class GEM_EXTERN GEMglReportError : public GemGLBase
{
	CPPEXTERN_HEADER(GEMglReportError, GemGLBase);

	public:
	  // Constructor
	  GEMglReportError (void);	// CON

	protected:
	  // Destructor
	  virtual ~GEMglReportError ();
	  // Do the rendering

	  virtual void render(GemState *state);


	private:
	  t_outlet *m_outlet;

	// static member functions
	  static void	 bangMessCallback (void*);
};
#endif // for header file
