/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    replace the alpha-channel of the 1st image by that of the 2nd image 

    Copyright (c) 1997-1998 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_TAKEALPHA_H_
#define _INCLUDE__GEM_PIXES_PIX_TAKEALPHA_H_

#include "Base/GemPixDualObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_takealpha
    
    Add two images together.

KEYWORDS
    pix
    
DESCRIPTION

-----------------------------------------------------------------*/
class GEM_EXTERN pix_takealpha : public GemPixDualObj
{
    CPPEXTERN_HEADER(pix_takealpha, GemPixDualObj);

    public:

	    //////////
    	// Constructor
    	pix_takealpha();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_takealpha();

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_RGBA(imageStruct &image, imageStruct &right);

    	//////////
    	// Do the processing
    	virtual void 	processRGBA_Gray(imageStruct &image, imageStruct &right);

	// MMX isn't really faster here
};

#endif	// for header file
