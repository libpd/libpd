/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    age an image

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

    this is based on EffecTV by Fukuchi Kentauro
    * Copyright (C) 2001 FUKUCHI Kentarou

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_YUV_H_
#define _INCLUDE__GEM_PIXES_PIX_YUV_H_

#include "Pixes/pix_rgba.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_yuv
    
    Change pix from "any" color-space to GL_YUV

KEYWORDS
    pix
    
DESCRIPTION
   
-----------------------------------------------------------------*/

class GEM_EXTERN pix_yuv : public pix_rgba
{
    CPPEXTERN_HEADER(pix_yuv, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_yuv();
    	
    protected:
};

#endif	// for header file
