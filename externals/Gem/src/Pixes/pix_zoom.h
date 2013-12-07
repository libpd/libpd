/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Change the pixel zooming for glDrawPixels

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_ZOOM_H_
#define _INCLUDE__GEM_PIXES_PIX_ZOOM_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_zoom
    
    Change the pixel zooming for glDrawPixels

KEYWORDS
    pix
    
DESCRIPTION
    
    Inlet for float, float - "zoom"

    "zoom" - the x and y mag
    
-----------------------------------------------------------------*/
class GEM_EXTERN pix_zoom : public GemBase
{
    CPPEXTERN_HEADER(pix_zoom, GemBase);

    public:

        //////////
        // Constructor
    	pix_zoom();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_zoom();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Turn back on pix_zoom test
    	virtual void 	postrender(GemState *state);

    	//////////
    	// x zoom
    	float	    	m_xZoom;

    	//////////
    	// y zoom
    	float	    	m_yZoom;

    	//////////
    	// Zoom mag
    	void	    	zoomMess(float xMag, float yMag);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	zoomMessCallback(void *data, t_floatarg xZoom, t_floatarg yZoom);
};

#endif	// for header file
