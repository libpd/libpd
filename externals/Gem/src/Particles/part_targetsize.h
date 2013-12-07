/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Set the target size for a particle system

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_TARGETSIZE_H_
#define _INCLUDE__GEM_PARTICLES_PART_TARGETSIZE_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_targetsize
    
	Set the target color for a particle system

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_targetsize : public partlib_base
{
    CPPEXTERN_HEADER(part_targetsize, partlib_base);

    public:

	    //////////
	    // Constructor
    	part_targetsize(t_floatarg size, t_floatarg scale);
    	
    	//////////
    	virtual void 	renderParticles(GemState *state);

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~part_targetsize();

    	//////////
    	// The scale factor angle
    	float	    	m_scale[3];

	//////////
	// The target size
	float		m_size[3];

    	//////////
    	// Scale changed
    	void	    	scaleMess(float x, float y, float z);
    	
    	//////////
    	// Size changed
    	void	    	sizeMess(float sizex, float sizey, float sizez);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	scaleMessCallback(void *data, t_symbol*, int, t_atom*);
    	static void 	sizeMessCallback (void *data, t_symbol*, int, t_atom*);
};

#endif	// for header file
