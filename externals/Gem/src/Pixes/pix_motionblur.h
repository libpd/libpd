/*
 *  pix_motionblur.h
 *  gem_darwin
 *
 *  Created by chris clepper on Mon Oct 07 2002.
 *  Copyright (c) 2002 __MyCompanyName__. All rights reserved.
 *
 */

#ifndef _INCLUDE__GEM_PIXES_PIX_MOTIONBLUR_H_ 
#define _INCLUDE__GEM_PIXES_PIX_MOTIONBLUR_H_ 

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_motionblur
    
    

KEYWORDS
    pix
    yuv
    
DESCRIPTION

  does motion blur by mixing the current and previous frames for a video 'feedback' effect
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_motionblur : public GemPixObj
{
CPPEXTERN_HEADER(pix_motionblur, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_motionblur();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_motionblur();


	void motionblurMessage(int, t_atom*);

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
      	virtual void 	processGrayImage(imageStruct &image);
  	virtual void 	processYUVImage(imageStruct &image);
#ifdef __MMX__
	virtual void    processMMX(imageStruct &image);

	virtual void    processRGBAMMX(imageStruct &image);
	virtual void    processYUVMMX(imageStruct &image);
	virtual void    processGrayMMX(imageStruct &image);
#endif

#ifdef __VEC__
        virtual void 	processYUVAltivec(imageStruct &image);
#endif
        
	imageStruct     m_savedImage;
        int		m_blur0, m_blur1;
        t_inlet         *inletmotionblur;

        
    private:
    
    	//////////
    	// Static member functions
        static void motionblurCallback       (void *data, t_symbol*,int,t_atom*);


};

#endif

