/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A curve

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_CURVE_H_
#define _INCLUDE__GEM_GEOS_CURVE_H_

#include "Geos/polygon.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    curve
    
    Creates a curve

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN curve : public polygon
{
    CPPEXTERN_HEADER(curve, polygon);

    public:

	    //////////
	    // Constructor
    	curve(t_floatarg numInputs);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~curve();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Resolution callback
    	void	    	resolutionMess(int res);
    	
    	//////////
    	// The rendering resolution
    	int 	    	m_resolution;

	GLfloat m_texCoords[4][2];

    private:
    	
    	//////////
    	// Static member functions
     	static void 	resolutionMessCallback(void *data, t_floatarg res);
};

#endif	// for header file
