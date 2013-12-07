/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    rotate a gem object

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_ROTATE_H_
#define _INCLUDE__GEM_MANIPS_ROTATE_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    rotate
    
    rotate a gem object

DESCRIPTION
    
    Inlet for a list - "vector"
    Inlet for a float - "ft1"

    "vector" - the vector of rotation
    "ft1" - the angle
    
-----------------------------------------------------------------*/
class GEM_EXTERN rotate : public GemBase
{
    CPPEXTERN_HEADER(rotate, GemBase);

    public:

        //////////
        // Constructor
    	rotate(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~rotate();

    	//////////
    	// When a gem message is received
    	virtual void	render(GemState *state);

    	//////////
    	// The rotation angle
    	float	    	m_angle;

    	//////////
    	// The rotation values (x, y, z)
    	float	    	m_vector[3];

    	//////////
    	// Angle changed
    	void	    	angleMess(float angle);
    	
    	//////////
    	// Vector changed
    	void	    	vectorMess(float x, float y, float z);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	angleMessCallback(void *data, t_floatarg angle);
    	static void 	vectorMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
};

#endif	// for header file
