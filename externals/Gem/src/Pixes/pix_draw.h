/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A pix_draw

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_DRAW_H_
#define _INCLUDE__GEM_PIXES_PIX_DRAW_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_draw
    
    Creates a pix_draw

KEYWORDS
    pix
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_draw : public GemBase
{
    CPPEXTERN_HEADER(pix_draw, GemBase);

    public:

	    //////////
	    // Constructor
    	pix_draw();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_draw();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
};

#endif	// for header file
