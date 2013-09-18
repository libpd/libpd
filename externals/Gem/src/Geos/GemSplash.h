/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A GemSplash

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_GEMSPLASH_H_
#define _INCLUDE__GEM_GEOS_GEMSPLASH_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemSplash
    
    Creates a GemSplash

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN GemSplash : public GemBase
{
    CPPEXTERN_HEADER(GemSplash, GemBase);

    public:

        //////////
        // Constructor
    	GemSplash();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~GemSplash();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
};

#endif	// for header file
