/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Send out a bang on pre and post render
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_CONTROLS_RENDER_TRIGGER_H_
#define _INCLUDE__GEM_CONTROLS_RENDER_TRIGGER_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    render_trigger
    
    Send out a bang on pre and post render

DESCRIPTION
    
    "render_triggerstate" - whether to use render_trigger blending
    
-----------------------------------------------------------------*/
class GEM_EXTERN render_trigger : public GemBase
{
    CPPEXTERN_HEADER(render_trigger, GemBase);

    public:

        //////////
        // Constructor
    	render_trigger();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~render_trigger();

    	//////////
    	// Push the current state
    	virtual void 	render(GemState *state);

    	//////////
    	// Pop the state
    	virtual void 	postrender(GemState *state);

    	//////////
        t_outlet        *m_preOut;	    // bang for the pre render

    	//////////
        t_outlet        *m_postOut;	    // bang for the post render 
};

#endif	// for header file
