/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Clamp pixel values to a threshold

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_THRESHOLD_H_
#define _INCLUDE__GEM_PIXES_PIX_THRESHOLD_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_threshold
    
    Clamp pixel values to a threshold

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vec_thresh"
    Inlet for a float - "ft1"
    
    "vec_thresh" - The threshold vector
    "ft1" - Set all thresholds to one value
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_threshold : public GemPixObj
{
    CPPEXTERN_HEADER(pix_threshold, GemPixObj);

    public:

        //////////
        // Constructor
    	pix_threshold();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_threshold();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	
    	//////////
    	// Do the processing
    	virtual void 	processGrayImage(imageStruct &image);
        	
        //////////
    	// Do the processing
    	virtual void 	processYUVImage(imageStruct &image);
		
#ifdef __VEC__
	//////////
    	// Do the processing
    	virtual void 	processYUVAltivec(imageStruct &image);
#endif  
    	//////////
    	// Set the new threshold vector
    	void	    	vecThreshMess(int argc, t_atom *argv);
    	
    	//////////
    	// Set the new threshold value
    	void	    	floatThreshMess(float thresh);
    	
    	//////////
    	// The new color
    	unsigned char  	m_thresh[4];
        unsigned char	m_Y;
    
    private:
    
    	//////////
    	// Static member functions
    	static void 	vecThreshMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
    	static void 	floatThreshMessCallback(void *data, t_floatarg thresh);
};

#endif	// for header file
