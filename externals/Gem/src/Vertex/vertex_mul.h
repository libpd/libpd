/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_combine

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_MUL_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_MUL_H_

#include "vertex_add.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_mul
    
    Creates a vertex_mul

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_mul : public vertex_add
{
    CPPEXTERN_HEADER(vertex_mul, vertex_add);

    public:

        //////////
        // Constructor
  vertex_mul(int, t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_mul(void);
	
    	//////////
    	// Do the rendering
	virtual void    vertexProcess(int lsize, float*larray, int rsize, float*rarray);
};

#endif	// for header file
