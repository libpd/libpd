/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A sphere

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_SPHERE_H_
#define _INCLUDE__GEM_GEOS_SPHERE_H_

#include "Base/GemGluObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    sphere
    
    Creates a sphere

KEYWORDS
    geo

DESCRIPTION
        
-----------------------------------------------------------------*/
class GEM_EXTERN sphere : public GemGluObj
{
    CPPEXTERN_HEADER(sphere, GemGluObj);

    public:

        //////////
        // Constructor
  sphere(t_floatarg size, t_floatarg slice=10.0);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~sphere();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

        virtual void  	createSphere(GemState *state);

        float		*m_x;
        float		*m_y;
        float		*m_z;
        int 		oldStacks, oldSlices;
        GLenum		oldDrawType;
	int             oldTexture;
        
};

#endif	// for header file
