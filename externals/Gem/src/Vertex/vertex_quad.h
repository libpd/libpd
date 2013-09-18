/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_quad

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_QUAD_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_QUAD_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_quad
    
    Creates a vertex_quad

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_quad : public GemBase
{
    CPPEXTERN_HEADER(vertex_quad, GemBase);

    public:

        //////////
        // Constructor
    	vertex_quad(void);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_quad(void);
        
        int	m_blend;
        float	*m_ColorArray;
        float	*m_VertexArray;
        float	*m_NormalArray;
        float	*m_TexCoordArray;

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
        static void 	blendMessCallback(void *data, t_floatarg size);

};

#endif	// for header file
