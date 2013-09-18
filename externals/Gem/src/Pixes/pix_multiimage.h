/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Load multiple images into a pix block

    Copyright (c) 1997-1999 Mark Danks
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_MULTIIMAGE_H_
#define _INCLUDE__GEM_PIXES_PIX_MULTIIMAGE_H_

#include <string.h>

#include "Base/GemBase.h"
#include "Gem/Image.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_multiimage
    
    Load multiple images into a pix block
    
KEYWORDS
    pix
    
DESCRIPTION

    You can select which file by giving a number.

-----------------------------------------------------------------*/
class GEM_EXTERN pix_multiimage : public GemBase
{
    CPPEXTERN_HEADER(pix_multiimage, GemBase);

    public:

	    //////////
	    // Constructor
    	pix_multiimage(t_symbol *filename, t_floatarg baseImage, t_floatarg topImage, t_floatarg skipRate);
    	
        class multiImageCache
        {
            public:
                
                multiImageCache(const char *_imageName)
                        : refCount(0), next(NULL), images(NULL), textBind(NULL),
                            numImages(0), baseImage(0), topImage(0), skipRate(0)
                        { imageName = strdup(_imageName); }
                ~multiImageCache()
                            { delete imageName;
                              for(int i=0; i < numImages;i++)
                              {
                        	    delete images[i];
                              }
							  delete [] textBind;
                              delete [] images;
                            }
                int                 refCount;
                multiImageCache     *next;
                imageStruct         **images;
                unsigned int		*textBind;
                int                 numImages;
                char                *imageName;
                int                 baseImage;
                int                 topImage;
                int                 skipRate;
        };
    	
    	//////////
        static multiImageCache  *s_imageCache;

    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_multiimage();

    	//////////
    	// When an open is received
    	virtual void	openMess(t_symbol *filename, int baseImage, int topImage, int skipRate);
    	
    	//////////
    	// Do the rendering
    	virtual void 	render(GemState *state);

    	//////////
    	// Clear the dirty flag on the pixBlock
    	virtual void 	postrender(GemState *state);

    	//////////
    	virtual void	startRendering();

    	//////////
    	// Change which image to display
    	void	    	changeImage(int imgNum);
    
    	//////////
    	// Clean up the images and the pixBlock
    	void	    	cleanImages();
    
	    //-----------------------------------
	    // GROUP:	Image data
	    //-----------------------------------
    
    	//////////
    	// The number of loaded images
    	int 	    	m_numImages;

    	//////////
    	// The current image
    	int 	    	m_curImage;

    	//////////
    	// The pixBlock with the current image
    	pixBlock    	m_pixBlock;
	imageStruct     m_imageStruct;

    	//////////
    	// The original images
        multiImageCache *m_loadedCache;
          	
    private:
    	
    	//////////
    	// static member functions
    	static void 	openMessCallback(void *data, t_symbol *filename, t_floatarg baseImage, t_floatarg topImage, t_floatarg skipRate);
    	static void 	changeImageCallback(void *data, t_floatarg imgNum);
};

#endif	// for header file
