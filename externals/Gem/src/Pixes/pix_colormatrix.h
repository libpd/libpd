/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Apply a color matrix kernel

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_COLORMATRIX_H_
#define _INCLUDE__GEM_PIXES_PIX_COLORMATRIX_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_colormatrix
    
    Apply a color matrix

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "matrix"
    
    "matrix" - The matrix for the color conversion
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_colormatrix : public GemPixObj
{
    CPPEXTERN_HEADER(pix_colormatrix, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_colormatrix();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_colormatrix();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
        
    	//////////
    	// Set the matrix
    	void	    	matrixMess(int argc, t_atom *argv);
    	
    	//////////
    	// The matrix
    	float  	    	m_matrix[16];
    
    private:
    
    	//////////
    	// Static member functions
    	static void 	matrixMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
};

#endif	// for header file
