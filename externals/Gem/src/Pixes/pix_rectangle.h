/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Create a pix-rectangle with a given RGB(A)

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

/////////////////////////////////////////////////////////
//
//  pix_rectangle
//
//  2002:forum::für::umläute:2000
//  iohannes m zmoelnig
//  mailto:zmoelnig@iem.mhsg.ac.at
//
/////////////////////////////////////////////////////////

#ifndef _INCLUDE__GEM_PIXES_PIX_RECTANGLE_H_
#define _INCLUDE__GEM_PIXES_PIX_RECTANGLE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_rectangle
    
    create a pix-rectangle with given RGB(A) 

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "coord"
    Inlet for a list - "color"

    "coord" - The coordinate-vector that defines the rect
	"color"  - the RGB(A)-color for the rectangle
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_rectangle : public GemPixObj
{
    CPPEXTERN_HEADER(pix_rectangle, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_rectangle();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_rectangle();

	  	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image);
	virtual void 	processYUVImage (imageStruct &image);
   
    	//////////
    	// Set the new color
    	void	    	vecColorMess(int argc, t_atom *argv);
    	
    	//////////
    	// Set the new coordinates
    	void	    	vecCoordMess(int argc, t_atom *argv);
    	
    	//////////
    	// The new color
	unsigned char	m_color[4];

	//////////
    	// The two corners
	int				m_lower_left[2];
	//////////
	int				m_upper_right[2];

    private:    
    	//////////
    	// Static member functions
    	static void 	vecColorMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
    	static void 	vecCoordMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
