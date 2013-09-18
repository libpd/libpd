/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Turn on/off depth test

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_DEPTH_H_
#define _INCLUDE__GEM_MANIPS_DEPTH_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    depth
    
    Turn on/off depth test

DESCRIPTION
    
    Inlet for a list - "depthstate"

    "depthstate" - whether to use the Z-buffer
    
-----------------------------------------------------------------*/
class GEM_EXTERN depth : public GemBase
{
    CPPEXTERN_HEADER(depth, GemBase);

    public:

	    //////////
	    // Constructor
  depth(int, t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~depth();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Turn back on depth test
    	virtual void 	postrender(GemState *state);

    	//////////
    	// Depth state
    	int	    		m_state;

    	//////////
    	// Depth changed
    	void	    	depthMess(int state);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	depthMessCallback(void *data, t_floatarg state);
};

#endif	// for header file
