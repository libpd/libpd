/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Add two images together.

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) GÂžnther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MIX_H_
#define _INCLUDE__GEM_PIXES_PIX_MIX_H_

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_mix
    
    Blends two images together.

KEYWORDS
    pix
    
DESCRIPTION

    gain $1 $2 - sets the gain for the left and right images.

-----------------------------------------------------------------*/
class GEM_EXTERN pix_mix : public GemPixDualObj
{
    CPPEXTERN_HEADER(pix_mix, GemPixDualObj);

    public:

	    //////////
    	// Constructor
    	pix_mix(int,t_atom*);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_mix();

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);
      	virtual void 	processGray_Gray(imageStruct &image, imageStruct &right);
    	virtual void 	processYUV_YUV(imageStruct &image, imageStruct &right);
#ifdef __MMX__
	virtual void 	processRGBA_MMX(imageStruct &image, imageStruct &right);
      	virtual void 	processGray_MMX(imageStruct &image, imageStruct &right);
    	virtual void 	processYUV_MMX(imageStruct &image, imageStruct &right);
#endif
#ifdef __VEC__
        virtual void 	processYUV_Altivec (imageStruct &image, imageStruct &right);
#endif
        
        virtual void 	gainMess (float X, float Y);
        
        long imageGain,rightGain;
        
         
         private:
    
    	//////////
    	// Static member functions
    	
        //static void gainCallback       (void *data, t_floatarg X, t_floatarg Y);
	static void gainCallback       (void *data, t_symbol*,int,t_atom*);
    
         

};

#endif	// for header file


