/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A polygon

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_POLYGON_H_
#define _INCLUDE__GEM_GEOS_POLYGON_H_

#include "Base/GemBase.h"
#include "Base/GemShape.h"


/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    polygon
    
    Creates a polygon

KEYWORDS
    geo
    
DESCRIPTION

    Inlet for a list - "vert_1"
    ...
    Inlet for a list - "vert_9"

    "list" - The first vertex
    "vert_1" - The second vertex
    ...
    "vert_9" - The tenth vertex
     
-----------------------------------------------------------------*/
class GEM_EXTERN polygon : public GemShape
{
    CPPEXTERN_HEADER(polygon, GemShape);

    public:

        //////////
        // Constructor
    	polygon(t_floatarg numInputs);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~polygon();

    	//////////
    	// Do the renderShapeing
    	virtual void 	renderShape(GemState *state);

     	//////////
    	// How the object should be drawn
    	virtual void	typeMess(t_symbol *type);

    	//////////
    	// Set the vertices
    	void	    	setVert(int whichOne, float x, float y, float z);

    	void	    	listMess(int,t_atom*);

	    //-----------------------------------
	    // GROUP:	Member variables
	    //-----------------------------------
    
    	//////////
    	// The vertices
			void createVertices(int);
    	int 	    	m_numVertices;
      float  *m_vertarray;
    	float **m_vert;

    	//////////
    	// The number of inlets (one per vertex)
    	int 	    	m_numInputs;
			t_inlet**m_inlet;

    private:
    	//////////
    	// Static member functions
    	static void 	typeMessCallback(void *data, t_symbol *type);
    	static void 	vertCallback(void *data, t_symbol *type, int argc, t_atom*argv);
			static void   vertexCallback(void *data, t_floatarg id, t_floatarg x, t_floatarg y, t_floatarg z);
    	static void 	listCallback(void *data, t_symbol *type, int argc, t_atom*argv);
};

#endif	// for header file
