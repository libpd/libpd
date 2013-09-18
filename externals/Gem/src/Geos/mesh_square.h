/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    A mesh_square

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_MESH_SQUARE_H_
#define _INCLUDE__GEM_GEOS_MESH_SQUARE_H_

#include "Base/GemShape.h"


#define MAXGRID 1000

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    mesh_square
    
    Creates a mesh_square

KEYWORDS
    geo
    
DESCRIPTION
    
-----------------------------------------------------------------*/
class GEM_EXTERN mesh_square : public GemShape
{
    CPPEXTERN_HEADER(mesh_square, GemShape);

    public:

        //////////
        // Constructor
    	mesh_square(t_floatarg sizeX,t_floatarg sizeY);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~mesh_square();
        

    	//////////
    	// Do the rendering
    	virtual void 	renderShape(GemState *state);
      static void 	gridMessCallback(void *data, t_floatarg size);
      static void 	gridXMessCallback(void *data, t_floatarg size);
      static void 	gridYMessCallback(void *data, t_floatarg size);


		//////////
		// getStuff
		int 		gridX, gridY;
		float		xsize, xsize0, ysize, ysize0;
		int 		alreadyInit;
		void		setSize( int valueX, int valueY );
		void            setGridX(int valX);
		void            setGridY(int valX);
		void		getTexCoords(void);
		float 		texCoords[MAXGRID][MAXGRID][2];
};

#endif	// for header file
