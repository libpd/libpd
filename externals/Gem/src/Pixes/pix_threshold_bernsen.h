/*-----------------------------------------------------------------
LOG
GEM - Graphics Environment for Multimedia

Clamp pixel values to a threshold_bernsen

Copyright (c) 1997-1998 Mark Danks. mark@danks.org
Copyright (c) Günther Geiger. geiger@epy.co.at
Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
Copyright (c) 2002 James Tittle & Chris Clepper
For information on usage and redistribution, and for a DISCLAIMER OF ALL
WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_THRESHOLD_BERNSEN_H_
#define _INCLUDE__GEM_PIXES_PIX_THRESHOLD_BERNSEN_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_threshold_bernsen
    
    Clamp pixel values to a threshold_bernsen

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vec_thresh"
    Inlet for a float - "ft1"
    
    "vec_thresh" - The threshold_bernsen vector
    "ft1" - Set all threshold_bernsens to one value
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_threshold_bernsen : public GemPixObj
{
    CPPEXTERN_HEADER(pix_threshold_bernsen, GemPixObj);

    public:

        //////////
        // Constructor
    	pix_threshold_bernsen();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_threshold_bernsen();

    	//////////
    	// Do the processing
        static void processGraySub_getMinMax(imageStruct&image,
                                      int fromX, int toX,
                                      int fromY, int toY,
                                      unsigned char*resultMin, 
                                      unsigned char*resultMax);
    	virtual void 	processGrayImage(imageStruct &image);
        	
    	//////////
    	// set the number of tiles
    	void	    	tilesMess(int w, int h);
    	void	    	contrastMess(int c);

        int m_xtiles, m_ytiles;

        int m_contrast;

        unsigned char*m_minVals;
        unsigned char*m_maxVals;

    
    private:
    
    	//////////
    	// Static member functions
    	static void 	tilesMessCallback(void *data, t_float w, t_float h);
    	static void 	contrastMessCallback(void *data, t_float c);
};

#endif	// for header file
