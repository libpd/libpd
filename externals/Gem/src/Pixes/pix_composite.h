/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Composite two pix images.

    Copyright (c) 1997-1998 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_COMPOSITE_H_
#define _INCLUDE__GEM_PIXES_PIX_COMPOSITE_H_

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_composite
    
    Composite two pix images.  Puts the second image over the top using
    alpha blending.

KEYWORDS
    pix
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_composite : public GemPixDualObj
{
    CPPEXTERN_HEADER(pix_composite, GemPixDualObj);

    public:

    //////////
    // Constructor
    pix_composite();
    	
 protected:
    	
    //////////
    // Destructor
    virtual ~pix_composite();

    //////////
    // Do the processing
    virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);
    virtual void 	processRGBA_Gray(imageStruct &image, imageStruct &right);
#ifdef __MMX__
    virtual void 	processRGBA_MMX(imageStruct &image, imageStruct &right);
#endif
};

#endif	// for header file
