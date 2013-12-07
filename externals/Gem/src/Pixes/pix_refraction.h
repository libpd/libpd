/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Copyright (c) 2003 James Tittle
    ported from pete's_plugins (www.petewarden.com)
    
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_REFRACTION_H_
#define _INCLUDE__GEM_PIXES_PIX_REFRACTION_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_refraction
    
    View pix thru glass blocks

KEYWORDS
    pix
    
DESCRIPTION

    
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_refraction : public GemPixObj
{
    CPPEXTERN_HEADER(pix_refraction, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_refraction();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_refraction();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
        virtual void	processYUVImage(imageStruct &image);

	imageStruct	myImage;
	int		nHeight;
	int		nWidth;
	int		init;

	U32*		pSource;
	U32*		pOutput;

	float 		m_Refraction;
	float 		m_CellWidth;
	float 		m_CellHeight;
	float 		m_DoAllowMagnification;
 
    private:
    
    	//////////
    	// Static member functions
    	static void 	refractCallback(void *data, t_floatarg m_Refraction);
	static void 	widthCallback(void *data, t_floatarg m_CellWidth);
	static void 	heightCallback(void *data, t_floatarg m_CellHeight);
	static void 	magCallback(void *data, t_floatarg m_DoAllowMagnification);
};

#endif	// for header file
