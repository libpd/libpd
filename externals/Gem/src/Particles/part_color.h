/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Set the color for a particle system

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PARTICLES_PART_COLOR_H_
#define _INCLUDE__GEM_PARTICLES_PART_COLOR_H_

#include "Particles/partlib_base.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS

	part_color
    
	Set the color for a particle system

DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN part_color : public partlib_base
{
    CPPEXTERN_HEADER(part_color, partlib_base);

    public:

	    //////////
	    // Constructor
    	part_color();
    	
    	//////////
    	virtual void 	renderParticles(GemState *state);

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~part_color();
	
	//////////
	// One end of the range
	float			m_color1[4];

	//////////
	// Other end of the range
	float			m_color2[4];

    	//////////
    	// Color changed
    	void	    	color1Mess(float red, float green, float blue, float alpha);
    	
    	//////////
    	// Color changed
    	void	    	color2Mess(float red, float green, float blue, float alpha);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	color1MessCallback(void *data, t_symbol*,int,t_atom*);
    	static void 	color2MessCallback(void *data, t_symbol*,int,t_atom*);
};

#endif	// for header file
