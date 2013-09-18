/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Pixel color inversion

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_INVERT_H_
#define _INCLUDE__GEM_PIXES_PIX_INVERT_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_invert
    
    Pixel color inversion

KEYWORDS
    pix
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_invert : public GemPixObj
{
    CPPEXTERN_HEADER(pix_invert, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_invert();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_invert();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image);
        virtual void 	processYUVImage (imageStruct &image);

#ifdef __MMX__
        virtual void 	processRGBAMMX(imageStruct &image);
        virtual void 	processYUVMMX (imageStruct &image);
        virtual void 	processGrayMMX(imageStruct &image);
#endif
#ifdef __VEC__
        virtual void 	processYUVAltivec  (imageStruct &image);
#endif
};

#endif	// for header file
