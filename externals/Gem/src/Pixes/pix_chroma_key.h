
/*
 *  pix_chroma_key.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_CHROMA_KEY_H_ 
#define _INCLUDE__GEM_PIXES_PIX_CHROMA_KEY_H_ 

#include "Base/GemPixDualObj.h"
#include "Utils/Functions.h"


/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_chroma_key
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

   chroma keying
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_chroma_key : public GemPixDualObj
{
CPPEXTERN_HEADER(pix_chroma_key, GemPixDualObj);

    public:

	    //////////
	    // Constructor
    	pix_chroma_key();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_chroma_key();

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);
    	
        //////////
    	// Do the YUV processing
    	virtual void 	processYUV_YUV(imageStruct &image, imageStruct &right);
   
#ifdef __MMX__
    	virtual void 	processRGBA_MMX(imageStruct &image, imageStruct &right);
      	virtual void 	processYUV_MMX(imageStruct &image, imageStruct &right);
  	virtual void 	processGray_MMX(imageStruct &image, imageStruct &right);
#endif
     
#ifdef __VEC__
        //////////
    	// Do the YUV Altivec processing
    	virtual void 	processYUV_Altivec(imageStruct &image, imageStruct &right);
#endif
        
        int m_direction,m_mode;
        unsigned char m_Yrange,m_Urange,m_Vrange,m_Yvalue,m_Uvalue,m_Vvalue;
        
    private:
    
    	//////////
    	// Static member functions
    	static void directionCallback       (void *data, t_floatarg state);
        static void modeCallback       (void *data, t_floatarg state);
        static void rangeCallback       (void *data, t_floatarg Yval, t_floatarg Uval,t_floatarg Vval);
        static void valueCallback       (void *data, t_floatarg Yval, t_floatarg Uval,t_floatarg Vval);

};

#endif

