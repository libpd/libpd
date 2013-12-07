/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A circle

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_CIRCLE_H_
#define _INCLUDE__GEM_GEOS_CIRCLE_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    circle
    
    Creates a circle

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN circle : public GemShape
{
    CPPEXTERN_HEADER(circle, GemShape);

    public:

        //////////
        // Constructor
        circle(t_floatarg size);
    	
    protected:
    	
        //////////
        // Destructor
        virtual ~circle();

        //////////
        // Do the renderShapeing
        virtual void 	renderShape(GemState *state);

        //////////
        // cos lookup table
        static GLfloat *m_cos;

        //////////
        // sin lookup table
        static GLfloat *m_sin;
};

#endif	// for header file
