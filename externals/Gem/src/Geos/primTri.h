/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A triangle primitive with 6 inlets

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_PRIMTRI_H_
#define _INCLUDE__GEM_GEOS_PRIMTRI_H_

#include "Base/GemBase.h"
#include "Base/GemShape.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    primtTri
    
    Creates a triangle primitive

KEYWORDS
    geo

DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN primTri : public GemShape
{
    CPPEXTERN_HEADER(primTri, GemShape);

    public:

        //////////
        // Constructor
    	primTri(t_floatarg size);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~primTri();

    	//////////
    	// Do the renderShapeing
    	virtual void 	renderShape(GemState *state);

    	//////////
    	// How the object should be drawn
    	virtual void	typeMess(t_symbol *type);

    	//////////
    	// Vector changed
    	void	    	vectMess(int which, float x, float y, float z)
			{ mVectors[which][0] = x; mVectors[which][1] = y; mVectors[which][2] = z; }

    	//////////
    	// Color changed
    	void	    	colMess(int which, float r, float g, float b, float a)
			{ mColors[which][0] = r; mColors[which][1] = g; mColors[which][2] = b; mColors[which][3] = a; }

		//////////
		// Vertex vectors
		float			mVectors[3][3];

		//////////
		// Vertex colors
		float			mColors[3][4];

	private:

       	//////////
    	// static member functions
    	static void 	vect1MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
    	static void 	vect2MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
    	static void 	vect3MessCallback(void *data, t_floatarg x, t_floatarg y, t_floatarg z);
    	static void 	col1MessCallback(void *data, t_symbol *, int argc, t_atom *argv);
    	static void 	col2MessCallback(void *data, t_symbol *, int argc, t_atom *argv);
    	static void 	col3MessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
