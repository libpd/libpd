/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A torus

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

/////////////////////////////////////////////////////////
// 1905:forum::für::umläute:2000
/////////////////////////////////////////////////////////
// added the gluTorus
/////////////////////////////////////////////////////////

#ifndef _INCLUDE__GEM_GEOS_TORUS_H_
#define _INCLUDE__GEM_GEOS_TORUS_H_

#include "Base/GemGluObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	torus
    
    Creates a torus

KEYWORD
    geo
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN torus : public GemGluObj
{
    CPPEXTERN_HEADER(torus, GemGluObj);

    public:

	    //////////
	    // Constructor
    	torus(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~torus();

    	//////////
		// Inner radius of the torus
		float			m_innerRadius;

    	//////////
		// Set the inner radius
		void			innerRadius(float radius);

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
	
	private:

    	//////////
    	// Static member functions
    	static void 	innerRadiusCallback(void *data, t_floatarg radius);
};

#endif	// for header file
