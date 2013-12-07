/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Multiply two images together.

    Copyright (c) 1997-1998 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MULTIPLY_H_
#define _INCLUDE__GEM_PIXES_PIX_MULTIPLY_H_

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_multiply
    
    Multiply two images together.

KEYWORDS
    pix
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_multiply : public GemPixDualObj
{
    CPPEXTERN_HEADER(pix_multiply, GemPixDualObj);

    public:

	    //////////
    	// Constructor
    	pix_multiply();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_multiply();

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);

    	//////////
    	//  This is called whenever a new image comes through.
		//	The left image is an RGBA, the right is a gray8
    	virtual void 	processRGBA_Gray(imageStruct &image, imageStruct &right); 	

    	//////////
    	//  This is called whenever a new image comes through.
		//	Both images are gray8
    	virtual void 	processGray_Gray(imageStruct &image, imageStruct &right);
        
    	//////////
    	// Do the processing
    	virtual void 	processYUV_YUV(imageStruct &image, imageStruct &right);

#ifdef __MMX__
     	virtual void 	processRGBA_MMX(imageStruct &image, imageStruct &right);
    	virtual void 	processYUV_MMX(imageStruct &image, imageStruct &right);
   	virtual void 	processGray_MMX(imageStruct &image, imageStruct &right);
#endif
        
};

#endif	// for header file
