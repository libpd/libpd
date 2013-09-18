/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    translate a gem object

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_TRANSLATE_H_
#define _INCLUDE__GEM_MANIPS_TRANSLATE_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    translate
    
    translate a gem object

DESCRIPTION
    
    Inlet for a list - "vector"
    Inlet for a float - "ft1"

    "vector" - the vector of translation
    "ft1" - the distance
    
-----------------------------------------------------------------*/
class GEM_EXTERN translate : public GemBase
{
    CPPEXTERN_HEADER(translate, GemBase);

    public:

        //////////
        // Constructor
    	translate(int argc, t_atom *argv);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~translate();

    	//////////
    	// When rendering occurs
    	virtual void	render(GemState *state);

    	//////////
    	// The translation vector (x, y, z)
    	float	    	m_vector[3];

    	//////////
    	// The translation distance
    	float	    	m_distance;

    	//////////
    	// Distance changed
    	void	    	distanceMess(float distance);
    	
    	//////////
    	// Vector changed
    	void	    	vectorMess(float x, float y, float z);
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	distanceMessCallback(void *data, t_floatarg distance);
    	static void 	vectorMessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
};

#endif	// for header file
