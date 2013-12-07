/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A cylinder

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_CYLINDER_H_
#define _INCLUDE__GEM_GEOS_CYLINDER_H_

#include "Base/GemGluObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	cylinder
    
    Creates a cylinder

KEYWORD
    geo
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN cylinder : public GemGluObj
{
    CPPEXTERN_HEADER(cylinder, GemGluObj);

    public:

	    //////////
	    // Constructor
  cylinder(t_floatarg size,t_floatarg slize);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~cylinder();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);


	virtual void setupParameters(void);
	GLdouble baseRadius;
	GLdouble topRadius;
	GLdouble height;
	GLint    slices;
	GLint    stacks;

};

#endif	// for header file
