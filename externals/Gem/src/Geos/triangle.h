/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A triangle

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_TRIANGLE_H_
#define _INCLUDE__GEM_GEOS_TRIANGLE_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    triangle
    
    Creates a triangle

KEYWORDS
    geo

DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN triangle : public GemShape
{
    CPPEXTERN_HEADER(triangle, GemShape);

    public:

        //////////
        // Constructor
    	triangle(t_floatarg size);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~triangle();

    	//////////
    	// Do the renderShapeing
    	virtual void 	renderShape(GemState *state);
};

#endif	// for header file
