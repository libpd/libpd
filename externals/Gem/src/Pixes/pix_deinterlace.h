/*
 *  pix_deinterlace.h
 *  GEM_darwin
 *
 *  Created by lincoln on 11/18/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */


#ifndef _INCLUDE__GEM_PIXES_PIX_DEINTERLACE_H_
#define _INCLUDE__GEM_PIXES_PIX_DEINTERLACE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_deinterlace
    
    Deinterlace a pix

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_deinterlace : public GemPixObj
{
    CPPEXTERN_HEADER(pix_deinterlace, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_deinterlace();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_deinterlace();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
		virtual void 	processYUVImage(imageStruct &image);
#ifdef __MMX__
 //       virtual void 	processRGBAMMX(imageStruct &image);
 //       virtual void 	processYUVMMX(imageStruct &image);
#endif
#ifdef __VEC__
    //    virtual void 	processRGBAAltivec(imageStruct &image);
        virtual void 	processYUVAltivec(imageStruct &image);
#endif
     	    	  
    	//////////
    	// Do the processing
        virtual void 	processGrayImage(imageStruct &image);
		
		int	m_mode;
		int	m_adaptive;

		imageStruct		m_savedImage;
		
	private:
	
		static void 	modeMessCallback(void *data, t_floatarg contrast);
		static void 	adaptiveMessCallback(void *data, t_floatarg contrast);
};



#endif	// for header file
