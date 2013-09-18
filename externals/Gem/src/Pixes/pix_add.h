/////////////////////////////////////////////////////////////////
//LOG
//    GEM - Graphics Environment for Multimedia
//
//    Add two images together.
//
//    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
//    Copyright (c) Günther Geiger. geiger@epy.co.at
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//    Copyright (c) 2002 James Tittle & Chris Clepper
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
//////////////////////////////////////////////////////////////

#ifndef _INCLUDE__GEM_PIXES_PIX_ADD_H_
#define _INCLUDE__GEM_PIXES_PIX_ADD_H_

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_add
    
    Add two images together.

KEYWORDS
    pix
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_add : public GemPixDualObj
{
    CPPEXTERN_HEADER(pix_add, GemPixDualObj);

    public:
      
	    //////////
    	// Constructor
    	pix_add();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_add();

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);
    	virtual void 	processRGBA_Gray(imageStruct &image, imageStruct &right);
    	virtual void 	processYUV_YUV(imageStruct &image, imageStruct &right);
#ifdef __MMX__
    	virtual void 	processRGBA_MMX(imageStruct &image, imageStruct &right);
    	virtual void 	processGray_MMX(imageStruct &image, imageStruct &right);
   	virtual void 	processYUV_MMX (imageStruct &image, imageStruct &right);
#endif
#ifdef __VEC__
	//////////
    	// altivec
    	virtual void 	processYUV_Altivec(imageStruct &image, imageStruct &right);
    	virtual void 	processRGBA_Altivec(imageStruct &image, imageStruct &right);
#endif

	virtual void    processDualImage(imageStruct &image, imageStruct &right);
};

#endif	// for header file
