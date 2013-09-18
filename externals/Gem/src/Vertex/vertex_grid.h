/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A vertex_grid

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_GRID_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_GRID_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_grid
    
    Creates a vertex_grid

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_grid : public GemBase
{
    CPPEXTERN_HEADER(vertex_grid, GemBase);

    public:

        //////////
        // Constructor
  vertex_grid(t_floatarg w, t_floatarg h);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_grid(void);
        
        int	m_x,m_y,m_oldx,m_oldy;
        float	m_spacex, m_spacey;
        float 	maxX,maxY;
        float 	ratioX, ratioY;
        float	*m_ColorArray;
        float	*m_VertexArray;
        float	*m_TexCoordArray;

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);
        static void 	sizeMessCallback(void *data, t_floatarg x, t_floatarg y);
        static void 	spacingMessCallback(void *data, t_floatarg x, t_floatarg y);

};

#endif	// for header file
