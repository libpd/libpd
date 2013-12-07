/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    diffuse a gem object

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_DIFFUSE_H_
#define _INCLUDE__GEM_MANIPS_DIFFUSE_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    diffuse
    
    diffuse a gem object

DESCRIPTION

    Inlet for a list - "diffuse"

    "diffuse" - the RGB diffuse to set the object to
    
-----------------------------------------------------------------*/
class GEM_EXTERN diffuse : public GemBase
{
    CPPEXTERN_HEADER(diffuse, GemBase);

    public:

	    //////////
	    // Constructor
    	diffuse(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~diffuse();

    	//////////
    	// Turn back on the color material
    	virtual void 	postrender(GemState *state);

    	//////////
    	// When a gem message is received
    	virtual void	render(GemState *state);

    	//////////
    	// The diffuse vector (RGBA)
    	float	    	m_diffuse[4];

    	//////////
    	// diffuse changed
    	void	    	diffuseMess(float red, float green, float blue, float alpha);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	diffuseMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
