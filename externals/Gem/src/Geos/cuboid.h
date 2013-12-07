/*-----------------------------------------------------------------
  LOG
  GEM - Graphics Environment for Multimedia

  A cuboid

  Copyright (c) 1997-1999 Mark Danks. mark@danks.org
  For information on usage and redistribution, and for a DISCLAIMER OF ALL
  WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

  modified cube geos into cuboid by erich berger 2001 rat@telecoma.net
  -----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_CUBOID_H_
#define _INCLUDE__GEM_GEOS_CUBOID_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
  -------------------------------------------------------------------
  CLASS
  cuboid
    
  Creates a cuboid

  KEYWORDS
  geo

  DESCRIPTION
    
  -----------------------------------------------------------------*/
class GEM_EXTERN cuboid : public GemShape
{
  CPPEXTERN_HEADER(cuboid, GemShape);

    public:

  //////////
  // Constructor
  cuboid(t_floatarg sizex, t_floatarg sizey, t_floatarg sizez);
 protected:
    	
  //////////
  // Destructor
  virtual ~cuboid();


	//////////
  // The height of the object
  void	    	heightMess(float sizey);

	//////////
  // The widht of the object
  void	    	widthMess(float sizez);

  //////////
  // Do the rendering
  virtual void 	renderShape(GemState *state);

  //////////
  // The height of the object
  GLfloat	    	m_sizey;

  //////////
  // The height inlet
  t_inlet         *m_inletY;

	//////////
  // The height of the object
  GLfloat	    	m_sizez;

  //////////
  // The height inlet
  t_inlet         *m_inletZ;
		
 private:

  //////////
  // Static member functions
  static void 	heightMessCallback(void *data, t_floatarg sizey);
  static void 	widthMessCallback(void *data, t_floatarg sizez);
};

#endif	// for header file
