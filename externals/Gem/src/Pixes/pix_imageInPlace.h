/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Load multiple images into a pix block and texture immediately

    Copyright (c) 1997-1999 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_IMAGEINPLACE_H_
#define _INCLUDE__GEM_PIXES_PIX_IMAGEINPLACE_H_

#include "Pixes/pix_multiimage.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_imageInPlace
    
    Load multiple images into a pix block and texture immediately
    
KEYWORDS
    pix
    
DESCRIPTION

    You can select which file by giving a number.

-----------------------------------------------------------------*/
class GEM_EXTERN pix_imageInPlace : public pix_multiimage
{
    CPPEXTERN_HEADER(pix_imageInPlace, pix_multiimage);

    public:

	    //////////
	    // Constructor
    	pix_imageInPlace(t_symbol *filename, t_floatarg baseImage, t_floatarg topImage, t_floatarg skipRate);
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_imageInPlace();

      ////////
      // extension check
      virtual bool isRunnable(void);

    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Do the rendering
    	virtual void 	postrender(GemState *state);

    	//////////
    	virtual void	startRendering();

    	//////////
    	virtual void	stopRendering();

    	//////////
    	// When a preload is received
    	virtual void	preloadMess(t_symbol *filename, int baseImage, int topImage, int skipRate);

    	//////////
    	// When a download is received
    	virtual void	downloadMess();
    	
    	//////////
    	// When a purge is received
    	virtual void	purgeMess();

    	//////////
    	// quality message
    	virtual void	textureQuality(int type);

    	//////////
    	// repeat message
    	virtual void	repeatMess(int type);
    	
	//////////
	int				mInPreload;
    GLuint          m_textureQuality, m_repeat;

    private:
    	
    	//////////
    	// static member functions
    	static void 	preloadMessCallback(void *data, t_symbol *filename, t_floatarg baseImage, t_floatarg topImage, t_floatarg skipRate);
    	static void 	downloadImageCallback(void *data);
    	static void 	purgeImageCallback(void *data);
    	static void 	textureMessCallback(void *data, t_floatarg type);
    	static void 	repeatMessCallback(void *data, t_floatarg type);
};

#endif	// for header file
