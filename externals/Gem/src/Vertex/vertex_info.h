/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_info

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_INFO_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_INFO_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_info
    
    Creates a vertex_info

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_info : public GemBase
{
    CPPEXTERN_HEADER(vertex_info, GemBase);

    public:

        //////////
        // Constructor
    	vertex_info(void);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_info(void);
        
        int 		m_previousSize;
        int		m_vertNum,m_vertCount;
        t_outlet	*m_Vsize;

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

 private:
        static void 	vertexMessCallback(void *data, t_floatarg num, t_floatarg counter);

};

#endif	// for header file
