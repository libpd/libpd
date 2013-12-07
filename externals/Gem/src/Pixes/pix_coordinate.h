/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    set the texture coordinates of a pixBlock

    Copyright (c) 1997-1998 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_COORDINATE_H_
#define _INCLUDE__GEM_PIXES_PIX_COORDINATE_H_

#include "Base/GemBase.h"

class TexCoord;

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_coordinate
    
    sets the texture coordinates

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "coords"
    "coords" - The texture coordinate list

-----------------------------------------------------------------*/
class GEM_EXTERN pix_coordinate : public GemBase
{
    CPPEXTERN_HEADER(pix_coordinate, GemBase);

    public:

	    //////////
	    // Constructor
    	pix_coordinate();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_coordinate();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Turn back off texture mapping
    	virtual void 	postrender(GemState *state);

    	//////////
    	// The texture coordinates
    	TexCoord    	*m_coords;
	// a place-holder, in case we want to scale the coords (for rectangle textures)
	TexCoord        *m_rectcoords;
    	
    	//////////
    	// The number
    	int 	    	m_numCoords;

    	int 	    	m_oldTexType;

	//////////
	// this is what we get from upstream
	TexCoord        *m_oldTexCoords;
	int             m_oldNumCoords;

    	//////////
    	// Set the texture coordinates
    	void	    	coordsMess(int argc, t_atom *argv);
    	
    private:
    
    	//////////
    	// Static member functions
    	static void 	coordsMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
