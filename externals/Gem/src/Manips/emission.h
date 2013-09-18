/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    emission a gem object

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_EMISSION_H_
#define _INCLUDE__GEM_MANIPS_EMISSION_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    emission
    
    emission a gem object

DESCRIPTION

    Inlet for a list - "emission"

    "emission" - the RGB emission to set the object to
    
-----------------------------------------------------------------*/
class GEM_EXTERN emission : public GemBase
{
    CPPEXTERN_HEADER(emission, GemBase);

    public:

	    //////////
	    // Constructor
    	emission(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~emission();

    	//////////
    	// Turn back on the color material
    	virtual void 	postrender(GemState *state);

    	//////////
    	// When a gem message is received
    	virtual void	render(GemState *state);

    	//////////
    	// The emission vector (RGBA)
    	float	    	m_emission[4];

    	//////////
    	// emission changed
    	void	    	emissionMess(float red, float green, float blue, float alpha);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	emissionMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
