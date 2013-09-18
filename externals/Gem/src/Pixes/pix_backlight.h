/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Copyright (c) 2003 James Tittle
    ported from pete's_plugins (www.petewarden.com)
    
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_BACKLIGHT_H_
#define _INCLUDE__GEM_PIXES_PIX_BACKLIGHT_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_backlight
    
    

KEYWORDS
    pix
    
DESCRIPTION

    
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_backlight : public GemPixObj
{
    CPPEXTERN_HEADER(pix_backlight, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_backlight();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_backlight();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
	virtual void	processYUVImage(imageStruct &image);
	//virtual void	processGrayImage(imageStruct &image);

	imageStruct	myImage;
	int		nHeight;
	int		nWidth;
	int		init;

	U32*		pSource;
	U32*		pOutput;
	
	float m_SpikeScale;
	float m_SpikeFloor;
	float m_SpikeCeiling;

	//int Pete_BackLight_Init();
	//void Pete_BackLight_DeInit();

    private:
    
    	//////////
    	// Static member functions
    	static void 	scaleCallback(void *data, t_floatarg m_SpikeScale);
	static void 	floorCallback(void *data, t_floatarg m_SpikeFloor);
	static void 	ceilingCallback(void *data, t_floatarg m_SpikeCeiling);
};

#endif	// for header file
