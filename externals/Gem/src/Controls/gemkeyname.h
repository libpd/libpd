/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Respond to mouse events
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_CONTROLS_GEMKEYNAME_H_
#define _INCLUDE__GEM_CONTROLS_GEMKEYNAME_H_

#include "Base/CPPExtern.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    gemkeyname
    
    Respond to keyboard events

DESCRIPTION
    
    
-----------------------------------------------------------------*/
class GEM_EXTERN gemkeyname : public CPPExtern
{
    CPPEXTERN_HEADER(gemkeyname, CPPExtern);

    public:

        //////////
        // Constructor
        gemkeyname();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~gemkeyname();

        //////////
        // keyname-button
        void            KeyNamePressed(char* string, int val, int state);

        //////////
        // The key-val outlet
        t_outlet    	*m_outKeyVal;

        //////////
        // The key-state outlet
        t_outlet    	*m_outKeyState;


    private:

        //////////
        // Static member functions
        static void     keynameCallback(char* x, int y, int z, void *data);
};

#endif  // for header file
