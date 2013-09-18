/*
 *  pix_blur.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_BLUR_H_ 
#define _INCLUDE__GEM_PIXES_PIX_BLUR_H_ 

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_blur
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

  old motion blur object.  soon to be an abstraction for pix_convolve ??
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_blur : public GemPixObj
{
CPPEXTERN_HEADER(pix_blur, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_blur();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_blur();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
      	virtual void 	processGrayImage(imageStruct &image);
    	virtual void 	processYUVImage(imageStruct &image);
        virtual void 	processYUVAltivec(imageStruct &image);
        
        unsigned int  *saved;
        int		m_blur;
         float		m_blurf;
        int		m_blurH,m_blurW,m_blurSize,m_blurBpp;
        t_inlet         *inletBlur;

        
    private:
    
    	//////////
    	// Static member functions
    	
        static void blurCallback       (void *data, t_floatarg value);


};

#endif

