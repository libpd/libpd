/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A colorSquare

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_COLORSQUARE_H_
#define _INCLUDE__GEM_GEOS_COLORSQUARE_H_

#include "Base/GemShape.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    colorSquare
    
    Creates a colorSquare

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN colorSquare : public GemShape
{
    CPPEXTERN_HEADER(colorSquare, GemShape);

    public:

        //////////
        // Constructor
    	colorSquare(t_floatarg size);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~colorSquare();

    	//////////
    	// Do the renderShapeing
    	virtual void 	renderShape(GemState *state);

    	//////////
    	// After renderShapeing
    	virtual void 	postrenderShape(GemState *state);

    	//////////
    	// Set the individual color vertices
        void            vertColorMess(int whichVert, float r, float g, float b);

        //////////
        // Color values
        float           m_color[4][3];

    private:
    
       	//////////
    	// static member functions
    	static void 	vert0MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b);
    	static void 	vert1MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b);
    	static void 	vert2MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b);
    	static void 	vert3MessCallback(void *data, t_floatarg r, t_floatarg g, t_floatarg b);
};

#endif	// for header file
