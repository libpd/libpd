/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Flip a pix

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    Copyright (c) 2002 James Tittle & Chris Clepper
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_FLIP_H_
#define _INCLUDE__GEM_PIXES_PIX_FLIP_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_flip
    
    Flip a pix

KEYWORDS
    pix
    
DESCRIPTION

    "horizontal" - Flip horizontally
    "vertical" - Flip vertically
    "both" - Flip in both directiosn
    "none" - Do nothing
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_flip : public GemPixObj
{
    CPPEXTERN_HEADER(pix_flip, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_flip();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_flip();

    	//////////
    	// Do the processing
       	virtual void 	processRGBAImage(imageStruct &image);
 	virtual void 	processGrayImage(imageStruct &image);
    	virtual void 	processYUVImage(imageStruct &image);

    	//////////
    	enum FlipType	{ HORIZONTAL, VERTICAL, BOTH, NONE };
    	
    	//////////
    	void	    	flipMess(FlipType type);

    	//////////
    	// The flip type
    	FlipType 	    m_flip;
    
    private:
    
    	//////////
    	// Static member functions
    	static void 	horMessCallback(void *data);
    	static void 	vertMessCallback(void *data);
    	static void 	bothMessCallback(void *data);
    	static void 	noneMessCallback(void *data);
	static void     flipMessCallback(void *data, t_symbol*s);
};

#endif	// for header file
