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

#ifndef _INCLUDE__GEM_VERTEX_VERTEX_ADD_H_
#define _INCLUDE__GEM_VERTEX_VERTEX_ADD_H_

#include "Base/GemVertex.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    vertex_add
    
    Creates a vertex_add

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN vertex_add : public GemBase
{
    CPPEXTERN_HEADER(vertex_add, GemBase);

    public:

        //////////
        // Constructor
  vertex_add(int, t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~vertex_add(void);
	
	// which left-hand array to process
	int m_leftType;

	// which right-hand array to process
	int m_rightType;

	// the right-hand array
        int	m_rightSize;
        float	*m_rightVertexArray;
        float	*m_rightColorArray;
	float   *m_rightTexCoordArray;
	float   *m_rightNormalArray;

        t_inlet *m_inlet;

	//////////
	// set the types
	void typeMess(int, t_atom*);

    	//////////
    	// Do the rendering
	virtual void    vertexProcess(int lsize, float*larray, int rsize, float*rarray);
    	virtual void 	render(GemState *state);
        virtual void 	postrender(GemState *state);
        virtual void 	rightRender(GemState *state);
        
        //private:
        static void 	gem_rightMessCallback(void *x, t_symbol *s, int argc, t_atom *argv);
        static void	typeMessCallback(void *data, t_symbol*s,int, t_atom*);
};

#endif	// for header file
