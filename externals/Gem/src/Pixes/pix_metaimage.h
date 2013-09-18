/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Copyright (c) 2003-2004 James Tittle
    non-yuv portions ported from pete's_plugins (www.petewarden.com)
    
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_METAIMAGE_H_
#define _INCLUDE__GEM_PIXES_PIX_METAIMAGE_H_

#include "Base/GemPixObj.h"

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_metaimage
    
    

KEYWORDS
    pix
    
DESCRIPTION

    
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_metaimage : public GemPixObj
{
    CPPEXTERN_HEADER(pix_metaimage, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_metaimage();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_metaimage();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
        virtual void	processYUVImage (imageStruct &image);
        virtual void	processGrayImage(imageStruct &image);

	imageStruct	myImage;
	int		nHeight;
	int		nWidth;
	int		init;

	U32*		pSource;
	U32*		pOutput;

	float 		m_Size;
	float 		m_DoDistanceBased;
	float 		m_DoCheapAndNasty;
	
	SPete_MemHandle 	hSubImage;

	int Pete_MetaImage_Init();
	void Pete_MetaImage_DeInit();

	U32 Pete_MetaImage_CreateSubImage(U32* pInput,U32* pSubImage,float SubWidth,float SubHeight);
	void Pete_MetaImage_DrawSubImages(U32* pSubImage,U32 AverageColour,float SubWidth,float SubHeight);
	void Pete_MetaImage_DrawSubImage(U32* pSource, U32* pShrunkBuffer,U32* pOutput, int nLeftX,int nTopY,int nRightX,int nBottomY,U32 WholeImageAverage,int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,U32 SubImageAverage);
	U32 Pete_MetaImage_GetAreaAverage(U32* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride);
	U32 Pete_MetaImage_ShrinkSourceImage(U32* pSource, U32* pOutput, float SubWidth,float SubHeight);
	U32 Pete_MetaImage_ShrinkSourceImageFast(U32* pSource, U32* pOutput, float SubWidth,float SubHeight);
	
	U32  CreateSubImageYUV(U32* pInput,U32* pSubImage,float SubWidth,float SubHeight);
	void DrawSubImagesYUV(U32* pSubImage,U32 AverageColour,float SubWidth,float SubHeight);
	void DrawSubImageYUV(U32* pSource, U32* pShrunkBuffer,U32* pOutput, int nLeftX,int nTopY,int nRightX,int nBottomY,U32 WholeImageAverage,int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,U32 SubImageAverage);
	U32  GetAreaAverageYUV(U32* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride);
	U32  ShrinkSourceImageYUV(U32* pSource, U32* pOutput, float SubWidth,float SubHeight);
	U32  ShrinkSourceImageFastYUV(U32* pSource, U32* pOutput, float SubWidth,float SubHeight);

	U8  CreateSubImageGray(U8* pInput,U8* pSubImage,float SubWidth,float SubHeight);
	void DrawSubImagesGray(U8* pSubImage,U8 AverageColour,float SubWidth,float SubHeight);
	void DrawSubImageGray(U8* pSource, U8* pShrunkBuffer,U8* pOutput, int nLeftX,int nTopY,int nRightX,int nBottomY,U8 WholeImageAverage,int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,U8 SubImageAverage);
	U8  GetAreaAverageGray(U8* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride);
	U8  ShrinkSourceImageGray(U8* pSource, U8* pOutput, float SubWidth,float SubHeight);
	U8  ShrinkSourceImageFastGray(U8* pSource, U8* pOutput, float SubWidth,float SubHeight);

	
    private:
    
    	//////////
    	// Static member functions
    	static void 	sizeCallback(void *data, t_floatarg sz);
	static void 	distanceCallback(void *data, t_floatarg m_DoDistanceBased);
	static void 	cheapCallback(void *data, t_floatarg m_DoCheapAndNasty);
};

#endif	// for header file
