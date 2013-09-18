/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Generate particles

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_VERTEX_H_
#define _INCLUDE__GEM_PARTICLES_PART_VERTEX_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_vertex
    
	Generate particles

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_vertex : public partlib_base
{
  CPPEXTERN_HEADER(part_vertex, partlib_base);

    public:

  //////////
  // Constructor
  part_vertex(t_floatarg x=0, t_floatarg y=0, t_floatarg z=0);
    	
  //////////
  virtual void 	renderParticles(GemState *state);

 protected:
    	
  //////////
  // Destructor
  virtual ~part_vertex();

  t_float m_x, m_y, m_z;		
  //////////
  void		posMess(t_float x, t_float y, t_float z)	{ m_x=x, m_y=y; m_z=z; }
	
 private:
	
  //////////
  // static member functions
  static void		posMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
};

#endif	// for header file
