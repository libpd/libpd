/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    accumulate a rotation

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_ACCUMROTATE_H_
#define _INCLUDE__GEM_MANIPS_ACCUMROTATE_H_

#include "Base/GemBase.h"
#include "Utils/Matrix.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	accumrotate
    
	accumulate a rotation

DESCRIPTION
    
    Inlet for X - "xVal"
    Inlet for Y - "yVal"
    Inlet for Z - "zVal"

    "reset" - set the matrix to identity
    
-----------------------------------------------------------------*/
class GEM_EXTERN accumrotate : public GemBase
{
    CPPEXTERN_HEADER(accumrotate, GemBase);

    public:

        //////////
        // Constructor
		accumrotate(int argc, t_atom *argv);
    	
	protected:
    	
    	//////////
    	// Destructor
    	virtual ~accumrotate();

    	//////////
    	// When a gem message is received
    	virtual void	render(GemState *state);

    	//////////
    	// The rotation matrix
		Matrix			m_rotMatrix;
				
    	//////////
    	// X value changed
    	void	    	xMess(float val);
    	
    	//////////
    	// Y value changed
    	void	    	yMess(float val);
    	
    	//////////
    	// Z value changed
    	void	    	zMess(float val);
    	
    	//////////
    	// A reset was received
    	void	    	reset();
    	
    private:
    	
    	//////////
    	// static member functions
    	static void 	xMessCallback(void *data, t_floatarg val);
    	static void 	yMessCallback(void *data, t_floatarg val);
    	static void 	zMessCallback(void *data, t_floatarg val);
    	static void 	resetCallback(void *data);
};

#endif	// for header file
