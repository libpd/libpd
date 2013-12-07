/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    normalize a pixBuf, so that the values are not between min..max but between 0..255
    MIN=min{R,G,B} and MAX=max{R,G,B} of all the pixBuf

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_NORMALIZE_H_
#define _INCLUDE__GEM_PIXES_PIX_NORMALIZE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_normalize
    
    Change the overall gain of a pix

KEYWORDS
    pix
    
DESCRIPTION

    Inlet for a list - "vec_gain"
    Inlet for a float - "ft1"

    "vec_gain" - The gain vector to set to
    "ft1" - Which bit to use as a mask (converted to an int)
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_normalize : public GemPixObj
{
    CPPEXTERN_HEADER(pix_normalize, GemPixObj);

      public:

    //////////
    // Constructor
    pix_normalize();
    	
 protected:
    	
    //////////
    // Destructor
    virtual ~pix_normalize();
    
    //////////
    // Do the processing
    virtual void 	processRGBAImage(imageStruct &image);
    virtual void 	processGrayImage(imageStruct &image);
    virtual void 	processYUVImage(imageStruct &image);
 
 private:
    
    //////////
    // Static member functions
    static void 	vecMaskMessCallback(void *data, t_symbol *, int argc, t_atom *argv);
    static void 	floatMaskMessCallback(void *data, t_floatarg gain);
};

#endif	// for header file
