/* ------------------------------------------------------------------
 * GEM - Graphics Environment for Multimedia
 *
 *  Copyright (c) 2002-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
 *	zmoelnig@iem.kug.ac.at
 *  For information on usage and redistribution, and for a DISCLAIMER
 *  OF ALL WARRANTIES, see the file, "GEM.LICENSE.TERMS"
 *
 * ------------------------------------------------------------------
 */

#ifndef _INCLUDE__GEM_CONTROLS_GEMLIST_MATRIX_H_
#define _INCLUDE__GEM_CONTROLS_GEMLIST_MATRIX_H_

#include "Base/GemBase.h"

/*
  CLASS
  gemlist_matrix
  KEYWORDS
  openGL	0
  DESCRIPTION
  get information (scale, shear, rotation, translation) about a gemlist
*/

class GEM_EXTERN gemlist_matrix : public GemBase
{
  CPPEXTERN_HEADER(gemlist_matrix, GemBase);

    public:
  // Constructor
  gemlist_matrix (t_floatarg);	// CON
 protected:
  // Destructor
  virtual ~gemlist_matrix ();
  // Do the rendering
  virtual void	render (GemState *state);
  // extension checks
  virtual bool isRunnable();

 private:
  // The outlets
  t_outlet    	*m_outletMatrice;
};
#endif // for header file
