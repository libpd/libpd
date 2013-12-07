/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Turn on polygon smoothing
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_POLYGON_SMOOTH_H_
#define _INCLUDE__GEM_MANIPS_POLYGON_SMOOTH_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    polygon_smooth
    
    Turn on polygon smoothing

DESCRIPTION
    
    "polygon_smoothstate" - whether to use polygon_smooth blending
    
-----------------------------------------------------------------*/
class GEM_EXTERN polygon_smooth : public GemBase
{
    CPPEXTERN_HEADER(polygon_smooth, GemBase);

    public:

        //////////
        // Constructor
    	polygon_smooth();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~polygon_smooth();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Turn off polygon_smooth blending
    	virtual void 	postrender(GemState *state);

    	//////////
    	// polygon_smooth blending state
    	int	    	    m_polygon_smoothState;
    	//////////
    	// Polygon_Smooth state changed
   	void	    	polygon_smoothMess(int polygon_smoothState);

   private:
    	
    	//////////
    	// static member functions
    	static void 	polygon_smoothMessCallback(void *data, t_floatarg polygon_smooth);
};

#endif	// for header file
