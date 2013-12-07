/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Push/pop the current OpenGL state
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org.
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_SEPARATOR_H_
#define _INCLUDE__GEM_MANIPS_SEPARATOR_H_

#include "Base/GemBase.h"
#include "Gem/State.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    separator
    
    Separates the effects of the rest of the chain from the what
    	happens below the separator.

DESCRIPTION
    
    separator does NOT save the state of the pixes, if they are
    	being used!
    
    "separatorstate" - whether to use separator blending
    
-----------------------------------------------------------------*/
class GEM_EXTERN separator : public GemBase
{
    CPPEXTERN_HEADER(separator, GemBase);

    public:

        //////////
        // Constructor
  separator(int, t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~separator();

    	//////////
    	// Push the current state
    	virtual void 	render(GemState *state);

    	//////////
    	// Pop the state
    	virtual void 	postrender(GemState *state);

    	//////////
    	// The current state
    	GemState    	m_state;

	bool m_pushed[4];
	bool m_active[4];
};

#endif	// for header file
