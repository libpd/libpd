/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Have the particles follow each other

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_FOLLOW_H_
#define _INCLUDE__GEM_PARTICLES_PART_FOLLOW_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_follow
    
	Have the particles follow each other

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_follow : public partlib_base
{
    CPPEXTERN_HEADER(part_follow, partlib_base);

    public:

	    //////////
	    // Constructor
    	part_follow(t_floatarg num);
    	
    	//////////
    	virtual void 	renderParticles(GemState *state);

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~part_follow();
		
		//////////
		void			numberMess(float num)	{ m_accel = num; }

		//////////
		float			m_accel;
	
	private:

		//////////
		// static member functions
		static void		numberMessCallback(void *data, t_floatarg num);
};

#endif	// for header file
