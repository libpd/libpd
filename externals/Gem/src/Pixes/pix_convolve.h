/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Apply a convolution kernel

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_CONVOLVE_H_
#define _INCLUDE__GEM_PIXES_PIX_CONVOLVE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_convolve
    
    Apply a convolution kernel

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "matrix"
    Inlet for a float - "ft1"
    
    "matrix" - The matrix for the convolution kernal
    "ft1" - The range of the matrix
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_convolve : public GemPixObj
{
    CPPEXTERN_HEADER(pix_convolve, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_convolve(t_floatarg row, t_floatarg col);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_convolve();

        void calculate3x3YUV(imageStruct &image,imageStruct &tempImg);
        void calculate3x3YUVAltivec(imageStruct &image,imageStruct &tempImg);
	void calculateRGBA3x3(imageStruct &image,imageStruct &tempImg);


    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image);
	virtual void 	processYUVImage(imageStruct &image);
    	
    	//////////
    	// Set the matrix range
    	void	    	rangeMess(float range);
    	
    	//////////
    	// Set the matrix
    	void	    	matrixMess(int argc, t_atom *argv);
    	
    	//////////
    	// The matrix
    	short int  	*m_imatrix;
    
    	//////////
    	// The range
    	int             m_irange;
	

    	//////////
    	// The number of rows
    	int 	    	m_rows;
    	
    	//////////
    	// The number of columns
    	int 	    	m_cols;
        
        int 		m_chroma;
    	
    private:
	imageStruct tempImg;
    
    	//////////
    	// Static member functions
    	static void 	rangeMessCallback(void *data, t_floatarg range);
    	static void 	matrixMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
        static void 	chromaMessCallback(void *data, t_floatarg value);
};

#endif	// for header file
