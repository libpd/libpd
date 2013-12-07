/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Base class for paths
    
    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_BASE_GEMPATHBASE_H_
#define _INCLUDE__GEM_BASE_GEMPATHBASE_H_

#include "Base/CPPExtern.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    GemPathBase
    

DESCRIPTION
    
    
-----------------------------------------------------------------*/
class GEM_EXTERN GemPathBase : public CPPExtern
{
    CPPEXTERN_HEADER(GemPathBase, CPPExtern);

    public:

        //////////
        // Constructor
    	GemPathBase(int argc, t_atom *argv);
   	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~GemPathBase();

    	//////////
    	// When an open is received
    	virtual void	openMess(t_symbol *arrayname);
    	
        //////////
        // When a float val is received
        virtual void    floatMess(float val) = 0;

        //////////
        // The number of dimensions
        int             m_numDimens;

        //////////
        // The array
        t_garray         *m_array;

        //////////
        // The outlet
        t_outlet        *m_out1;

    private:
    	
    	//////////
    	// static member functions
    	static void 	openMessCallback(void *data, t_symbol *arrayname);
        static void     floatMessCallback(void *data, float n);
};

#endif	// for header file
