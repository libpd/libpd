/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Orbit around a point

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_ORBITPOINT_H_
#define _INCLUDE__GEM_PARTICLES_PART_ORBITPOINT_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_orbitpoint
    
	Orbit around a point

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_orbitpoint : public partlib_base
{
    CPPEXTERN_HEADER(part_orbitpoint, partlib_base);

    public:

	    //////////
	    // Constructor
    	part_orbitpoint(t_floatarg val1, t_floatarg val2, t_floatarg val3, t_floatarg grav);
    	
    	//////////
    	virtual void 	renderParticles(GemState *state);

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~part_orbitpoint();
		
		//////////
		void			vectorMess(float val1, float val2, float val3)
							{ m_pos[0] = val1; m_pos[1] = val2; m_pos[2] = val3; }

		//////////
		void			numberMess(float num)	{ m_grav = num; }

		//////////
		float			m_pos[3];
	
		//////////
		float			m_grav;
	
	private:

		//////////
		// static member functions
		static void		numberMessCallback(void *data, t_floatarg num);
		static void		vectorMessCallback(void *data, t_floatarg val1, t_floatarg val2, t_floatarg val3);
};

#endif	// for header file
