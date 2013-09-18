/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Use orthogonal viewing

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_MANIPS_ORTHO_H_
#define _INCLUDE__GEM_MANIPS_ORTHO_H_

#include "Base/GemBase.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
	ortho
    
	Use orthogonal viewing

DESCRIPTION
    
    Inlet for a list - "orthostate"

    "orthostate" - whether to go orthogonal
    
-----------------------------------------------------------------*/
class GEM_EXTERN ortho : public GemBase
{
    CPPEXTERN_HEADER(ortho, GemBase);

    public:

	    //////////
	    // Constructor
    	ortho();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~ortho();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Turn back on depth test
    	virtual void 	postrender(GemState *state);

    	//////////
    	// Ortho state
    	int	    		m_state;

    	//////////
    	// Ortho changed
    	void	    	orthoMess(int state);

    	//////////
    	// Compatibility
    	int	    		m_compat;
    	void	    	compatMess(int state);

    	
    private:
    	
    	//////////
    	// static member functions
     	static void 	orthoMessCallback(void *data, t_floatarg state);
   	static void 	compatMessCallback(void *data, t_floatarg state);
};

#endif	// for header file
