/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Apply gravity to particles

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_GRAVITY_H_
#define _INCLUDE__GEM_PARTICLES_PART_GRAVITY_H_

#include "papi.h"

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_gravity
    
	Apply gravity to particles

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_gravity : public partlib_base
{
    CPPEXTERN_HEADER(part_gravity, partlib_base);

    public:

	    //////////
	    // Constructor
    	part_gravity(t_floatarg val1, t_floatarg val2, t_floatarg val3);
    	
    	//////////
    	virtual void 	renderParticles(GemState *state);

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~part_gravity();

    	//////////
    	// Vector changed
    	void	    	vectorMess(float x, float y, float z);
    	
		//////////
		float			m_vector[3];

    private:
    	
    	//////////
    	// static member functions
    	static void 	vectorMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
};

#endif	// for header file
