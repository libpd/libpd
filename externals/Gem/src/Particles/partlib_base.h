/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Draw a partlib_base group

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PARTLIB_BASE_H_
#define _INCLUDE__GEM_PARTICLES_PARTLIB_BASE_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	partlib_base
    
	Base class of all the part_* objects

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN partlib_base : public GemBase
{
 public:

  //////////
  // Constructor
  partlib_base(void);
 	
  //////////
  // Destructor
  virtual ~partlib_base();
    	
  //////////
  virtual void 	render(GemState *);
   
  virtual void renderParticles(GemState*)=0;

 protected:
  float m_tickTime;

};

#endif	// for header file
