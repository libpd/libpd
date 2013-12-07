/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A spline path
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_SPLINE_PATH_H_
#define _INCLUDE__GEM_MANIPS_SPLINE_PATH_H_

#include "Base/GemPathBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    spline_path
    

DESCRIPTION
    
    
-----------------------------------------------------------------*/
class GEM_EXTERN spline_path : public GemPathBase
{
    CPPEXTERN_HEADER(spline_path, GemPathBase);

    public:

        //////////
        // Constructor
    	spline_path(int argc, t_atom *argv);
   	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~spline_path();

        //////////
        // When a float val is received
        virtual void    floatMess(float val);
};

#endif	// for header file
