/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    map an image to polygons

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_GEOS_IMAGEVERT_H_
#define _INCLUDE__GEM_GEOS_IMAGEVERT_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    imageVert
    
    map an image to polygons

DESCRIPTION
    
	Notice that even though imageVert is derived from GemPixObj, we
		still override a lot of the virtual functions...including
		render.

-----------------------------------------------------------------*/
class GEM_EXTERN imageVert : public GemPixObj
{
    CPPEXTERN_HEADER(imageVert, GemPixObj);

    public:

	    //////////
	    // Constructor
    	imageVert();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~imageVert();

    	//////////
    	// This is a dummy function for us
    	virtual void 	processImage(imageStruct &) { }

    	//////////
    	// Do the processing for an RGBA image
		// [in] image - The image to use
		// [in] texture - Is texture mapping turned on?
    	virtual void 	processRGBAPix(imageStruct &image, int texture);

    	//////////
    	// Do the processing for gray8 image
		// [in] image - The image to use
		// [in] texture - Is texture mapping turned on?
    	virtual void 	processGrayPix(imageStruct &image, int texture);

    	//////////
    	// Do the processing for YUV image
		// [in] image - The image to use
		// [in] texture - Is texture mapping turned on?
    	virtual void 	processYUVPix(imageStruct &image, int texture);
        
    	//////////
    	// Do the rendering.
    	virtual void 	render(GemState *state);

        //////////
        // The display list
        GLuint          m_dispList;

        //////////
        // Do we need to rebuild the display list?
        int             m_rebuildList;
};

#endif	// for header file
