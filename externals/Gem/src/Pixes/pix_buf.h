/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    buffer a pixBlock

    Copyright (c) 1997-1998 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_BUF_H_
#define _INCLUDE__GEM_PIXES_PIX_BUF_H_

#include "Base/GemBase.h"
#include "Gem/Image.h"

class GemCache;

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_buf
    
    Creates a pix_buf

KEYWORDS
    pix
    
DESCRIPTION
    This makes an assumption that the size of the image doesn't
    	change once the rendering has started    

-----------------------------------------------------------------*/
class GEM_EXTERN pix_buf : public GemBase
{
    CPPEXTERN_HEADER(pix_buf, GemBase);

    public:

	    //////////
	    // Constructor
    	pix_buf(t_floatarg);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_buf();

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// After the rendering
    	virtual void 	postrender(GemState *state);

    	//////////
    	// Clean everything up
    	void	    	cleanImage();
    	   	
	//////////
	// the pixBlock-cache
	pixBlock    cachedPixBlock;
	pixBlock    *orgPixBlock;

	//////////
	// force output of the buffer:
	void            bangMess();
	bool            m_banged;
	
	void            autoMess(int);
	bool            m_auto;


private:
	
	//////////
	// Static member callbacks
	static void bangMessCallback(void *);
	static void autoMessCallback(void *, t_floatarg);
};

#endif	// for header file
