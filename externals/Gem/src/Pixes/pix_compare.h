/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia


    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2003 Daniel Heckenberg

    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_COMPARE_H_ 
#define _INCLUDE__GEM_PIXES_PIX_COMPARE_H_ 

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_compare
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

   compares the pixel values of two images
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_compare : public GemPixDualObj
{
CPPEXTERN_HEADER(pix_compare, GemPixDualObj);

    public:

	    //////////
	    // Constructor
    	pix_compare();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_compare();

    	
    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);
	virtual void 	processYUV_YUV  (imageStruct &image, imageStruct &right);
	virtual void 	processGray_Gray(imageStruct &image, imageStruct &right);

#ifdef __MMX__
	virtual void 	processYUV_MMX (imageStruct &image, imageStruct &right);
	virtual void 	processGray_MMX(imageStruct &image, imageStruct &right);
#endif
#ifdef __VEC__
        //////////
    	// Do the Altivec processing
    	virtual void 	processYUV_Altivec(imageStruct &image, imageStruct &right);
#endif
        
        int m_direction;
        
    private:
    
    	//////////
    	// Static member functions
    	static void directionCallback       (void *data, t_floatarg state);


};

#endif

