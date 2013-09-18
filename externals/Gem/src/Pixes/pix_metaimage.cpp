////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
// non-yuv portions derived from pete's_plugins (www.petewarden.com)
//
// Implementation file
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_metaimage.h"
	
CPPEXTERN_NEW(pix_metaimage);

/////////////////////////////////////////////////////////
//
// pix_refraction
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_metaimage :: pix_metaimage()
{ 
    m_Size = 0.2f;
    m_DoDistanceBased = 0.0f;
    m_DoCheapAndNasty = 0.0f;
    hSubImage = NULL;

    init =0;
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("size"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_metaimage :: ~pix_metaimage()
{ 
    Pete_MetaImage_DeInit();
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_metaimage :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	Pete_MetaImage_Init();
	init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    float SubWidth;
    float SubHeight;
    
    m_Size = clampFunc(m_Size,0.0f,1.0f);

    if (m_DoDistanceBased>0.0f) {

		const float Distance=1.0f+(m_Size*(nHeight-1.0f));

		SubWidth=nWidth/Distance;
		SubHeight=nHeight/Distance;

    } else {

		SubWidth=1+(m_Size*(nWidth-1));
		SubHeight=1+(m_Size*(nHeight-1));
    }

    U32* pSubImageData=static_cast<U32*>(Pete_LockHandle(hSubImage));
    if (pSubImageData==NULL) {
		return;
    } 

    U32 AverageColour=Pete_MetaImage_CreateSubImage(pSource,pSubImageData,SubWidth,SubHeight);

    Pete_MetaImage_DrawSubImages(pSubImageData,AverageColour,SubWidth,SubHeight);

    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_metaimage :: processYUVImage(imageStruct &image)
{
    nWidth = image.xsize/2;
    nHeight = image.ysize;
    if (!init) {
	Pete_MetaImage_Init();
	init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    //    myImage.setBlack();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    float SubWidth;
    float SubHeight;
    
    m_Size = clampFunc(m_Size,0.0f,1.0f);

    if (m_DoDistanceBased>0.0f) {

		const float Distance=1.0f+(m_Size*(nHeight-1.0f));

		SubWidth=nWidth/Distance;
		SubHeight=nHeight/Distance;

    } else {

		SubWidth=1+(m_Size*(nWidth-1));
		SubHeight=1+(m_Size*(nHeight-1));
    }

    U32* pSubImageData=static_cast<U32*>(Pete_LockHandle(hSubImage));
    if (pSubImageData==NULL) {
		return;
    } 

    U32 AverageColour = CreateSubImageYUV(pSource,pSubImageData,SubWidth,SubHeight);

    DrawSubImagesYUV(pSubImageData,AverageColour,SubWidth,SubHeight);

    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_metaimage :: processGrayImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	Pete_MetaImage_Init();
	init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    //    myImage.setBlack();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    float SubWidth;
    float SubHeight;
    
    m_Size = clampFunc(m_Size,0.0f,1.0f);

    if (m_DoDistanceBased>0.0f) {
      const float Distance=1.0f+(m_Size*(nHeight-1.0f));
      SubWidth=nWidth/Distance;
      SubHeight=nHeight/Distance;
    } else {
      SubWidth=1+(m_Size*(nWidth-1));
      SubHeight=1+(m_Size*(nHeight-1));
    }

    U32* pSubImageData=static_cast<U32*>(Pete_LockHandle(hSubImage));
    if (pSubImageData==NULL)return;

    U8 AverageColour = CreateSubImageGray((U8*)pSource,(U8*)pSubImageData,SubWidth,SubHeight);

    DrawSubImagesGray((U8*)pSubImageData,AverageColour,SubWidth,SubHeight);

    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// do the other processing here
//
/////////////////////////////////////////////////////////
int pix_metaimage :: Pete_MetaImage_Init() {

	Pete_MetaImage_DeInit();

	const int nNumPixels=(nWidth*nHeight);
	const int nNumBytes=(nNumPixels*sizeof(U32));
	hSubImage = Pete_NewHandle(nNumBytes);
	if (hSubImage==NULL) {
		Pete_MetaImage_DeInit();
		return 0;
	}

	return 1;

}

void pix_metaimage :: Pete_MetaImage_DeInit() {
  if (hSubImage!=NULL) {
    Pete_FreeHandle(hSubImage);
    hSubImage=NULL;
  }
}

U32 pix_metaimage :: Pete_MetaImage_CreateSubImage(U32* pInput,U32* pSubImage,float SubWidth,float SubHeight) {
  U32 AverageColour;

  if (m_DoCheapAndNasty>0.0f) {
    AverageColour=
      Pete_MetaImage_ShrinkSourceImageFast(pInput,pSubImage,SubWidth,SubHeight);
  } else {
    AverageColour=
      Pete_MetaImage_ShrinkSourceImage(pInput,pSubImage,SubWidth,SubHeight);
  }
  return AverageColour;
}

void pix_metaimage :: Pete_MetaImage_DrawSubImages(U32* pSubImage,U32 AverageColour,float SubWidth,float SubHeight) {
  const int nHalfWidth=nWidth/2;
  const int nHalfHeight=nHeight/2;

  float CentreX=nHalfWidth+(SubWidth/2);
  float CentreY=nHalfHeight+(SubHeight/2);

  int nSubImageCountX=static_cast<int>(CentreX/SubWidth);
  int nSubImageCountY=static_cast<int>(CentreY/SubHeight);

  const float StartX=(nHalfWidth-(SubWidth/2))-(nSubImageCountX*SubWidth);
  const float EndX=(nHalfWidth+(SubWidth/2))+(nSubImageCountX*SubWidth);
  const float StartY=(nHalfHeight-(SubHeight/2))-(nSubImageCountY*SubHeight);
  const float EndY=(nHalfHeight+(SubHeight/2))+(nSubImageCountY*SubHeight);

  float CurrentY;
  for (CurrentY=StartY; CurrentY<EndY; CurrentY+=SubHeight) {
    float CurrentX;
    for (CurrentX=StartX; CurrentX<EndX; CurrentX+=SubWidth) {
      const int nLeftX=static_cast<int>(CurrentX);
      const int nTopY=static_cast<int>(CurrentY);
      const int nRightX=static_cast<int>(CurrentX+SubWidth);
      const int nBottomY=static_cast<int>(CurrentY+SubHeight);
      const int nClippedLeftX=clampFunc(nLeftX,0,(nWidth-1));
      const int nClippedTopY=clampFunc(nTopY,0,(nHeight-1));
      const int nClippedRightX=clampFunc(nRightX,0,(nWidth-1));
      const int nClippedBottomY=clampFunc(nBottomY,0,(nHeight-1));

      U32 SubImageAverage=Pete_MetaImage_GetAreaAverage(
							pSource,
							nClippedLeftX,
							nClippedTopY,
							nClippedRightX,
							nClippedBottomY,
							4);

      Pete_MetaImage_DrawSubImage(
				  pSource,
				  pSubImage,
				  pOutput,
				  nLeftX,
				  nTopY,
				  nRightX,
				  nBottomY,
				  AverageColour,
				  nClippedLeftX,
				  nClippedTopY,
				  nClippedRightX,
				  nClippedBottomY,
				  SubImageAverage);
    }
  }
}

void pix_metaimage :: Pete_MetaImage_DrawSubImage(U32* pSource, U32* pShrunkBuffer,U32* pOutput,int nLeftX,int nTopY,int nRightX,int nBottomY,U32 WholeImageAverage,int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,U32 SubImageAverage) {
  //const int nWidth=pInstanceData->nWidth;
  //const int nHeight=pInstanceData->nHeight;
  const int nSubRed=(SubImageAverage>>SHIFT_RED)&0xff;
  const int nSubGreen=(SubImageAverage>>SHIFT_GREEN)&0xff;
  const int nSubBlue=(SubImageAverage>>SHIFT_BLUE)&0xff;
  const int nSubAlpha=(SubImageAverage>>SHIFT_ALPHA)&0xff;

  const int nWholeRed=(WholeImageAverage>>SHIFT_RED)&0xff;
  const int nWholeGreen=(WholeImageAverage>>SHIFT_GREEN)&0xff;
  const int nWholeBlue=(WholeImageAverage>>SHIFT_BLUE)&0xff;
  const int nWholeAlpha=(WholeImageAverage>>SHIFT_ALPHA)&0xff;

  const int nRedDelta=(nSubRed-nWholeRed);
  const int nGreenDelta=(nSubGreen-nWholeGreen);
  const int nBlueDelta=(nSubBlue-nWholeBlue);
  const int nAlphaDelta=(nSubAlpha-nWholeAlpha);


  const int nXDelta=nClippedRightX-nClippedLeftX;
  const int nYDelta=nClippedBottomY-nClippedTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return;
  
  U32* pCurrentSource=pShrunkBuffer;

  pCurrentSource+=((nClippedTopY-nTopY)*nWidth);
  pCurrentSource+=(nClippedLeftX-nLeftX);

  U32* pCurrentOutput=pOutput+(nClippedTopY*nWidth)+nClippedLeftX;
  U32*const pOutputEnd=(pCurrentOutput+(nYDelta*nWidth)+nXDelta);

  while (pCurrentOutput<pOutputEnd) {		
    U32*const pOutputLineStart=pCurrentOutput;
    U32*const pOutputLineEnd=pCurrentOutput+nXDelta;
    U32* pSourceLineStart=pCurrentSource;

    while (pCurrentOutput<pOutputLineEnd) {
      const U32 SourceColour=*pCurrentSource;
      const U32 nSourceRed=(SourceColour>>SHIFT_RED)&0xff;
      const U32 nSourceGreen=(SourceColour>>SHIFT_GREEN)&0xff;
      const U32 nSourceBlue=(SourceColour>>SHIFT_BLUE)&0xff;
      const U32 nSourceAlpha=(SourceColour>>SHIFT_ALPHA)&0xff;

      const U32 nOutputRed=clampFunc(nSourceRed+nRedDelta,0,255);
      const U32 nOutputGreen=clampFunc(nSourceGreen+nGreenDelta,0,255);
      const U32 nOutputBlue=clampFunc(nSourceBlue+nBlueDelta,0,255);
      const U32 nOutputAlpha=clampFunc(nSourceAlpha+nAlphaDelta,0,255);//0xff;

      const U32 OutputColour=
	((nOutputRed&0xff)<<SHIFT_RED)|
	((nOutputGreen&0xff)<<SHIFT_GREEN)|
	((nOutputBlue&0xff)<<SHIFT_BLUE)|
	((nOutputAlpha&0xff)<<SHIFT_ALPHA);

      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
      pCurrentSource+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
    pCurrentSource=pSourceLineStart+nWidth;
  }
}

U32 pix_metaimage :: Pete_MetaImage_GetAreaAverage(U32* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride) {

  //const int nWidth=pInstanceData->nWidth;
  //const int nHeight=pInstanceData->nHeight;

  const int nXDelta=nRightX-nLeftX;
  const int nYDelta=nBottomY-nTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return 0x00000000;
	
  U32* pCurrentImage=pImage+(nTopY*nWidth)+nLeftX;
  U32*const pImageEnd=(pCurrentImage+(nYDelta*nWidth)+nXDelta);
  
  int nRedTotal=0;
  int nGreenTotal=0;
  int nBlueTotal=0;
  int nAlphaTotal=0;

  int nSampleCount=0;

  while (pCurrentImage<pImageEnd) {		
    U32*const pImageLineStart=pCurrentImage;
    U32*const pImageLineEnd=pCurrentImage+nXDelta;

    while (pCurrentImage<pImageLineEnd) {
      const U32 ImageColour=*pCurrentImage;

      const U32 nImageRed=(ImageColour>>SHIFT_RED)&0xff;
      const U32 nImageGreen=(ImageColour>>SHIFT_GREEN)&0xff;
      const U32 nImageBlue=(ImageColour>>SHIFT_BLUE)&0xff;
      const U32 nImageAlpha=(ImageColour>>SHIFT_ALPHA)&0xff;

      nRedTotal+=nImageRed;
      nGreenTotal+=nImageGreen;
      nBlueTotal+=nImageBlue;
      nAlphaTotal+=nImageAlpha;
   
      nSampleCount+=1;
      
      pCurrentImage+=nStride;
    }

    pCurrentImage=pImageLineStart+(nStride*nWidth);
  }

  const int nAverageRed=(nRedTotal/nSampleCount);
  const int nAverageGreen=(nGreenTotal/nSampleCount);
  const int nAverageBlue=(nBlueTotal/nSampleCount);
  const int nAverageAlpha=(nAlphaTotal/nSampleCount);


  U32 Average=
    (nAverageRed<<SHIFT_RED)|
    (nAverageGreen<<SHIFT_GREEN)|
    (nAverageBlue<<SHIFT_BLUE)|
    (nAverageAlpha<<SHIFT_ALPHA);

  return Average;
}

U32 pix_metaimage :: Pete_MetaImage_ShrinkSourceImage(U32* pSource, U32* pOutput, float SubWidth,float SubHeight) {
  
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nRedTotal=0;
  int nGreenTotal=0;
  int nBlueTotal=0;
  int nAlphaTotal=0;

  int nSampleCount=0;

  U32* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {

    U32* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    int nBottomY=static_cast<int>(SourceY+SourceYInc);
    nBottomY=clampFunc(nBottomY,0,(nHeight-1));

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {

      const int nLeftX=static_cast<int>(SourceX);
      int nRightX=static_cast<int>(SourceX+SourceXInc);
      nRightX=clampFunc(nRightX,0,(nWidth-1));

      const U32 OutputColour=
	Pete_MetaImage_GetAreaAverage(pSource,nLeftX,nTopY,nRightX,nBottomY,1);

      const U32 nOutputRed=(OutputColour>>SHIFT_RED)&0xff;
      const U32 nOutputGreen=(OutputColour>>SHIFT_GREEN)&0xff;
      const U32 nOutputBlue=(OutputColour>>SHIFT_BLUE)&0xff;
      const U32 nOutputAlpha=(OutputColour>>SHIFT_ALPHA)&0xff;

      nRedTotal+=nOutputRed;
      nGreenTotal+=nOutputGreen;
      nBlueTotal+=nOutputBlue;
      nAlphaTotal+=nOutputAlpha;

      nSampleCount+=1;

      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const int nAverageRed=(nRedTotal/nSampleCount);
  const int nAverageGreen=(nGreenTotal/nSampleCount);
  const int nAverageBlue=(nBlueTotal/nSampleCount);
  const int nAverageAlpha=(nAlphaTotal/nSampleCount);

  U32 Average=
    (nAverageRed<<SHIFT_RED)|
    (nAverageGreen<<SHIFT_GREEN)|
    (nAverageBlue<<SHIFT_BLUE)|
    (nAverageAlpha<<SHIFT_ALPHA);

  return Average;
}

U32 pix_metaimage :: Pete_MetaImage_ShrinkSourceImageFast(U32* pSource, U32* pOutput, float SubWidth,float SubHeight) {
  
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nRedTotal=0;
  int nGreenTotal=0;
  int nBlueTotal=0;
  int nAlphaTotal=0;

  int nSampleCount=0;

  U32* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {
    U32* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    U32* pSourceLineStart=pSource+(nTopY*nWidth);

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {
      const int nLeftX=static_cast<int>(SourceX);
      const U32 OutputColour=*(pSourceLineStart+nLeftX);
      const U32 nOutputRed=(OutputColour>>SHIFT_RED)&0xff;
      const U32 nOutputGreen=(OutputColour>>SHIFT_GREEN)&0xff;
      const U32 nOutputBlue=(OutputColour>>SHIFT_BLUE)&0xff;
      const U32 nOutputAlpha=(OutputColour>>SHIFT_ALPHA)&0xff;

      nRedTotal+=nOutputRed;
      nGreenTotal+=nOutputGreen;
      nBlueTotal+=nOutputBlue;
      nAlphaTotal+=nOutputAlpha;

      nSampleCount+=1;
      *pCurrentOutput=OutputColour;
      pCurrentOutput+=1;
    }
    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const int nAverageRed=(nRedTotal/nSampleCount);
  const int nAverageGreen=(nGreenTotal/nSampleCount);
  const int nAverageBlue=(nBlueTotal/nSampleCount);
  const int nAverageAlpha=(nAlphaTotal/nSampleCount);

  U32 Average=
    (nAverageRed<<SHIFT_RED)|
    (nAverageGreen<<SHIFT_GREEN)|
    (nAverageBlue<<SHIFT_BLUE)|
    (nAverageAlpha<<SHIFT_ALPHA);

  return Average;
}


/////////////////////////////////////////////////////////
// YUV processing
//
/////////////////////////////////////////////////////////
U32 pix_metaimage :: CreateSubImageYUV(U32* pInput,U32* pSubImage,float SubWidth,float SubHeight)
{
  U32 AverageColour;

  if (m_DoCheapAndNasty>0.0f) {
    AverageColour = ShrinkSourceImageFastYUV(pInput,pSubImage,SubWidth,SubHeight);
  } else {
    AverageColour = ShrinkSourceImageYUV(pInput,pSubImage,SubWidth,SubHeight);
  }
  return AverageColour;
}

void pix_metaimage :: DrawSubImagesYUV(U32* pSubImage,U32 AverageColour,float SubWidth,float SubHeight)
{
  const int nHalfWidth=nWidth/2;
  const int nHalfHeight=nHeight/2;

  float CentreX=nHalfWidth+(SubWidth/2);
  float CentreY=nHalfHeight+(SubHeight/2);

  int nSubImageCountX=static_cast<int>(CentreX/SubWidth);
  int nSubImageCountY=static_cast<int>(CentreY/SubHeight);

  const float StartX=(nHalfWidth-(SubWidth/2))-(nSubImageCountX*SubWidth);
  const float EndX=(nHalfWidth+(SubWidth/2))+(nSubImageCountX*SubWidth);
  const float StartY=(nHalfHeight-(SubHeight/2))-(nSubImageCountY*SubHeight);
  const float EndY=(nHalfHeight+(SubHeight/2))+(nSubImageCountY*SubHeight);

  float CurrentY;
  for (CurrentY=StartY; CurrentY<EndY; CurrentY+=SubHeight) {
    float CurrentX;
    for (CurrentX=StartX; CurrentX<EndX; CurrentX+=SubWidth) {
      const int nLeftX=static_cast<int>(CurrentX);
      const int nTopY=static_cast<int>(CurrentY);
      const int nRightX=static_cast<int>(CurrentX+SubWidth);
      const int nBottomY=static_cast<int>(CurrentY+SubHeight);
      const int nClippedLeftX=clampFunc(nLeftX,0,(nWidth-1));
      const int nClippedTopY=clampFunc(nTopY,0,(nHeight-1));
      const int nClippedRightX=clampFunc(nRightX,0,(nWidth-1));
      const int nClippedBottomY=clampFunc(nBottomY,0,(nHeight-1));

      U32 SubImageAverage = GetAreaAverageYUV(
							pSource,
							nClippedLeftX,
							nClippedTopY,
							nClippedRightX,
							nClippedBottomY,
							4);

      DrawSubImageYUV(
				  pSource,
				  pSubImage,
				  pOutput,
				  nLeftX,
				  nTopY,
				  nRightX,
				  nBottomY,
				  AverageColour,
				  nClippedLeftX,
				  nClippedTopY,
				  nClippedRightX,
				  nClippedBottomY,
				  SubImageAverage);
    }
  }
}

void pix_metaimage :: DrawSubImageYUV(U32* pSource, U32* pShrunkBuffer,U32* pOutput,
				       int nLeftX,int nTopY,int nRightX,int nBottomY,U32 WholeImageAverage,
				       int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,
				       U32 SubImageAverage)
{
  const int nSubU =(SubImageAverage>>SHIFT_U)&0xff;
  const int nSubY1=(SubImageAverage>>SHIFT_Y1)&0xff;
  const int nSubV =(SubImageAverage>>SHIFT_V)&0xff;
  const int nSubY2=(SubImageAverage>>SHIFT_Y2)&0xff;

  const int nWholeU =(WholeImageAverage>>SHIFT_U)&0xff;
  const int nWholeY1=(WholeImageAverage>>SHIFT_Y1)&0xff;
  const int nWholeV =(WholeImageAverage>>SHIFT_V)&0xff;
  const int nWholeY2=(WholeImageAverage>>SHIFT_Y2)&0xff;

  const int nUDelta  = (nSubU  - nWholeU);
  const int nY1Delta = (nSubY1 - nWholeY1);
  const int nVDelta  = (nSubV  - nWholeV);
  const int nY2Delta = (nSubY2 - nWholeY2);


  const int nXDelta=nClippedRightX-nClippedLeftX;
  const int nYDelta=nClippedBottomY-nClippedTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return;
  
  U32* pCurrentSource=pShrunkBuffer;

  pCurrentSource+=((nClippedTopY-nTopY)*nWidth);
  pCurrentSource+=(nClippedLeftX-nLeftX);

  U32* pCurrentOutput=pOutput+(nClippedTopY*nWidth)+nClippedLeftX;
  U32*const pOutputEnd=(pCurrentOutput+(nYDelta*nWidth)+nXDelta);

  while (pCurrentOutput<pOutputEnd) {		
    U32*const pOutputLineStart=pCurrentOutput;
    U32*const pOutputLineEnd=pCurrentOutput+nXDelta;
    U32* pSourceLineStart=pCurrentSource;

    while (pCurrentOutput<pOutputLineEnd) {
      const U32 SourceColour=*pCurrentSource;
      const U32 nSourceU =(SourceColour>>SHIFT_U )&0xff;
      const U32 nSourceY1=(SourceColour>>SHIFT_Y1)&0xff;
      const U32 nSourceV =(SourceColour>>SHIFT_V )&0xff;
      const U32 nSourceY2=(SourceColour>>SHIFT_Y2)&0xff;

      const U32 nOutputU =clampFunc(nSourceU +nUDelta ,0,255);
      const U32 nOutputY1=clampFunc(nSourceY1+nY1Delta,0,255);
      const U32 nOutputV =clampFunc(nSourceV +nVDelta ,0,255);
      const U32 nOutputY2=clampFunc(nSourceY2+nY2Delta,0,255);//0xff;

      const U32 OutputColour=
	((nOutputU &0xff)<<SHIFT_U)|
	((nOutputY1&0xff)<<SHIFT_Y1)|
	((nOutputV &0xff)<<SHIFT_V)|
	((nOutputY2&0xff)<<SHIFT_Y2);

      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
      pCurrentSource+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
    pCurrentSource=pSourceLineStart+nWidth;
  }
}

U32 pix_metaimage :: GetAreaAverageYUV(U32* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride)
{
  const int nXDelta=nRightX-nLeftX;
  const int nYDelta=nBottomY-nTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return 0x00000000;
	
  U32* pCurrentImage=pImage+(nTopY*nWidth)+nLeftX;
  U32*const pImageEnd=(pCurrentImage+(nYDelta*nWidth)+nXDelta);
  
  int nYTotal=0;
  int nUTotal=0;
  int nVTotal=0;

  int nSampleCount=0;

  while (pCurrentImage<pImageEnd) {		
    U32*const pImageLineStart=pCurrentImage;
    U32*const pImageLineEnd=pCurrentImage+nXDelta;

    while (pCurrentImage<pImageLineEnd) {
      const U32 ImageColour=*pCurrentImage;

      const U32 nImageU =(ImageColour>>SHIFT_U )&0xff;
      const U32 nImageY1=(ImageColour>>SHIFT_Y1)&0xff;
      const U32 nImageV =(ImageColour>>SHIFT_V )&0xff;
      const U32 nImageY2=(ImageColour>>SHIFT_Y2)&0xff;

      nUTotal+=nImageU;
      nYTotal+=nImageY1;
      nVTotal+=nImageV;
      nYTotal+=nImageY2;
   
      nSampleCount+=1;
      
      pCurrentImage+=nStride;
    }

    pCurrentImage=pImageLineStart+(nStride*nWidth);
  }
  const unsigned char nAverageU=(nUTotal/nSampleCount);
  const unsigned char nAverageV=(nVTotal/nSampleCount);
  const unsigned char nAverageY=(nYTotal/(nSampleCount<<1));

  U32 Average=
    (nAverageU<<SHIFT_U)|
    (nAverageY<<SHIFT_Y1)|
    (nAverageV<<SHIFT_V)|
    (nAverageY<<SHIFT_Y2);

  return Average;
}

U32 pix_metaimage :: ShrinkSourceImageYUV(U32* pSource, U32* pOutput, float SubWidth,float SubHeight) {
 
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nUTotal=0;
  int nYTotal=0;
  int nVTotal=0;

  int nSampleCount=0;

  U32* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {

    U32* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    int nBottomY=static_cast<int>(SourceY+SourceYInc);
    nBottomY=clampFunc(nBottomY,0,(nHeight-1));

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {

      const int nLeftX=static_cast<int>(SourceX);
      int nRightX=static_cast<int>(SourceX+SourceXInc);
      nRightX=clampFunc(nRightX,0,(nWidth-1));

      const U32 OutputColour = GetAreaAverageYUV(pSource,nLeftX,nTopY,nRightX,nBottomY,1);

      const U32 nOutputU  = (OutputColour>>SHIFT_U )&0xff;
      const U32 nOutputY1 = (OutputColour>>SHIFT_Y1)&0xff;
      const U32 nOutputV  = (OutputColour>>SHIFT_V )&0xff;
      const U32 nOutputY2 = (OutputColour>>SHIFT_Y2)&0xff;

      nUTotal+=nOutputU;
      nYTotal+=nOutputY1;
      nVTotal+=nOutputV;
      nYTotal+=nOutputY2;

      nSampleCount+=1;

      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const int nAverageU=(nUTotal/nSampleCount);
  const int nAverageY=(nYTotal/(nSampleCount<<1));
  const int nAverageV=(nVTotal/nSampleCount);
  //const int nAverageY2=(nY2Total/nSampleCount);

  U32 Average=
    (nAverageU<<SHIFT_U)|
    (nAverageY<<SHIFT_Y1)|
    (nAverageV<<SHIFT_V)|
    (nAverageY<<SHIFT_Y2);

  return Average;
}

U32 pix_metaimage :: ShrinkSourceImageFastYUV(U32* pSource, U32* pOutput, float SubWidth,float SubHeight)
{
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nUTotal=0;
  int nYTotal=0;
  int nVTotal=0;

  int nSampleCount=0;

  U32* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {
    U32* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    U32* pSourceLineStart=pSource+(nTopY*nWidth);

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {
      const int nLeftX=static_cast<int>(SourceX);
      const U32 OutputColour=*(pSourceLineStart+nLeftX);
      const U32 nOutputU =(OutputColour>>SHIFT_U )&0xff;
      const U32 nOutputY1=(OutputColour>>SHIFT_Y1)&0xff;
      const U32 nOutputV =(OutputColour>>SHIFT_V )&0xff;
      const U32 nOutputY2=(OutputColour>>SHIFT_Y2)&0xff;

      nUTotal+=nOutputU;
      nYTotal+=nOutputY1;
      nVTotal+=nOutputV;
      nYTotal+=nOutputY2;

      nSampleCount+=1;
      *pCurrentOutput=OutputColour;
      pCurrentOutput+=1;
    }
    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const unsigned char nAverageY=(nYTotal/(nSampleCount<<1));
  const unsigned char nAverageU=(nUTotal/nSampleCount);
  const unsigned char nAverageV=(nVTotal/nSampleCount);

  U32 Average=
    (nAverageU<<SHIFT_U )|
    (nAverageY<<SHIFT_Y1)|
    (nAverageV<<SHIFT_V )|
    (nAverageY<<SHIFT_Y2);

  return Average;
}


/////////////////////////////////////////////////////////
// Gray processing
//
/////////////////////////////////////////////////////////
U8 pix_metaimage :: CreateSubImageGray(U8*pInput,U8*pSubImage,float SubWidth,float SubHeight)
{
  U8 AverageColour;

  if (m_DoCheapAndNasty>0.0f) {
    AverageColour = ShrinkSourceImageFastGray(pInput,pSubImage,SubWidth,SubHeight);
  } else {
    AverageColour = ShrinkSourceImageGray(pInput,pSubImage,SubWidth,SubHeight);
  }
  return AverageColour;
}

void pix_metaimage :: DrawSubImagesGray(U8* pSubImage,U8 AverageColour,float SubWidth,float SubHeight)
{
  const int nHalfWidth=nWidth/2;
  const int nHalfHeight=nHeight/2;

  float CentreX=nHalfWidth+(SubWidth/2);
  float CentreY=nHalfHeight+(SubHeight/2);

  int nSubImageCountX=static_cast<int>(CentreX/SubWidth);
  int nSubImageCountY=static_cast<int>(CentreY/SubHeight);

  const float StartX=(nHalfWidth-(SubWidth/2))-(nSubImageCountX*SubWidth);
  const float EndX=(nHalfWidth+(SubWidth/2))+(nSubImageCountX*SubWidth);
  const float StartY=(nHalfHeight-(SubHeight/2))-(nSubImageCountY*SubHeight);
  const float EndY=(nHalfHeight+(SubHeight/2))+(nSubImageCountY*SubHeight);

  float CurrentY;
  for (CurrentY=StartY; CurrentY<EndY; CurrentY+=SubHeight) {
    float CurrentX;
    for (CurrentX=StartX; CurrentX<EndX; CurrentX+=SubWidth) {
      const int nLeftX=static_cast<int>(CurrentX);
      const int nTopY=static_cast<int>(CurrentY);
      const int nRightX=static_cast<int>(CurrentX+SubWidth);
      const int nBottomY=static_cast<int>(CurrentY+SubHeight);
      const int nClippedLeftX=clampFunc(nLeftX,0,(nWidth-1));
      const int nClippedTopY=clampFunc(nTopY,0,(nHeight-1));
      const int nClippedRightX=clampFunc(nRightX,0,(nWidth-1));
      const int nClippedBottomY=clampFunc(nBottomY,0,(nHeight-1));

      U8 SubImageAverage = GetAreaAverageGray(
					      (U8*)pSource,
					      nClippedLeftX,
					      nClippedTopY,
					      nClippedRightX,
					      nClippedBottomY,
					      1);


      DrawSubImageGray(
		       (U8*)pSource,
		       (U8*)pSubImage,
		       (U8*)pOutput,
		       nLeftX,
		       nTopY,
		       nRightX,
		       nBottomY,
		       AverageColour,
		       nClippedLeftX,
		       nClippedTopY,
		       nClippedRightX,
		       nClippedBottomY,
		       SubImageAverage);
    }
  }
}

void pix_metaimage :: DrawSubImageGray(U8* pSource, U8* pShrunkBuffer,U8* pOutput,
				       int nLeftX,int nTopY,int nRightX,int nBottomY,U8 WholeImageAverage,
				       int nClippedLeftX,int nClippedTopY,int nClippedRightX,int nClippedBottomY,
				       U8 SubImageAverage)
{
  const int nSubU =(SubImageAverage>>SHIFT_U)&0xff;
  const int nWholeU =(WholeImageAverage>>SHIFT_U)&0xff;
  const int nUDelta  = (nSubU  - nWholeU);

  const int nXDelta=nClippedRightX-nClippedLeftX;
  const int nYDelta=nClippedBottomY-nClippedTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return;
  
  U8*pCurrentSource=(U8*)pShrunkBuffer;

  pCurrentSource+=((nClippedTopY-nTopY)*nWidth);
  pCurrentSource+=(nClippedLeftX-nLeftX);

  U8*pCurrentOutput=pOutput+(nClippedTopY*nWidth)+nClippedLeftX;
  U8*const pOutputEnd=(pCurrentOutput+(nYDelta*nWidth)+nXDelta);

  while (pCurrentOutput<pOutputEnd) {		
    U8*const pOutputLineStart=pCurrentOutput;
    U8*const pOutputLineEnd=pCurrentOutput+nXDelta;
    U8*pSourceLineStart=pCurrentSource;

    while (pCurrentOutput<pOutputLineEnd) {
      const U8 SourceColour=*pCurrentSource;
      const U8 OutputColour =clampFunc(SourceColour+nUDelta ,0,255);
      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
      pCurrentSource+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
    pCurrentSource=pSourceLineStart+nWidth;
  }
}

U8 pix_metaimage :: GetAreaAverageGray(U8* pImage,int nLeftX,int nTopY,int nRightX,int nBottomY,int nStride)
{
  const int nXDelta=nRightX-nLeftX;
  const int nYDelta=nBottomY-nTopY;

  if ((nXDelta<=0)||(nYDelta<=0))return 0x00000000;
	
  U8* pCurrentImage=pImage+(nTopY*nWidth)+nLeftX;
  U8*const pImageEnd=(pCurrentImage+(nYDelta*nWidth)+nXDelta);
  
  int nYTotal=0;
  int nSampleCount=0;

  while (pCurrentImage<pImageEnd) {		
    U8*const pImageLineStart=pCurrentImage;
    U8*const pImageLineEnd=pCurrentImage+nXDelta;

    while (pCurrentImage<pImageLineEnd) {
      const U8 ImageColour=*pCurrentImage;

      nYTotal+=ImageColour;

      nSampleCount++;
      pCurrentImage+=nStride;
    }

    pCurrentImage=pImageLineStart+(nStride*nWidth);
  }
  const U8 Average=nYTotal/nSampleCount;

  return Average;
}

U8 pix_metaimage :: ShrinkSourceImageGray(U8* pSource, U8* pOutput, float SubWidth,float SubHeight) {
 
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nYTotal=0;

  int nSampleCount=0;

  U8* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {

    U8* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    int nBottomY=static_cast<int>(SourceY+SourceYInc);
    nBottomY=clampFunc(nBottomY,0,(nHeight-1));

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {

      const int nLeftX=static_cast<int>(SourceX);
      int nRightX=static_cast<int>(SourceX+SourceXInc);
      nRightX=clampFunc(nRightX,0,(nWidth-1));

      const U8 OutputColour = GetAreaAverageGray(pSource,nLeftX,nTopY,nRightX,nBottomY,1);

      nYTotal+=OutputColour;

      nSampleCount+=1;

      *pCurrentOutput=OutputColour;

      pCurrentOutput+=1;
    }

    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const U8 Average=nYTotal/nSampleCount;

  return Average;
}

U8 pix_metaimage :: ShrinkSourceImageFastGray(U8* pSource, U8* pOutput, float SubWidth,float SubHeight)
{
  if (SubWidth>static_cast<float>(nWidth))  SubWidth=static_cast<float>(nWidth);
  if (SubHeight>static_cast<float>(nHeight))SubHeight=static_cast<float>(nHeight);

  const float SourceYInc=(nHeight/SubHeight);
  const float SourceXInc=(nWidth/SubWidth);

  int nYTotal=0;

  int nSampleCount=0;

  U8* pCurrentOutput=pOutput;
	
  float SourceY;
  for (SourceY=0.0f; SourceY<nHeight; SourceY+=SourceYInc) {
    U8* pOutputLineStart=pCurrentOutput;
    const int nTopY=static_cast<int>(SourceY);
    U8* pSourceLineStart=pSource+(nTopY*nWidth);

    float SourceX;
    for (SourceX=0.0f; SourceX<nWidth; SourceX+=SourceXInc) {
      const int nLeftX=static_cast<int>(SourceX);
      const U8 OutputColour=*(pSourceLineStart+nLeftX);

      nYTotal+=OutputColour;

      nSampleCount+=1;
      *pCurrentOutput=OutputColour;
      pCurrentOutput+=1;
    }
    pCurrentOutput=pOutputLineStart+nWidth;
  }

  const U8 Average=nYTotal/nSampleCount;

  return Average;
}


/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_metaimage :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_metaimage::sizeCallback),
		  gensym("size"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_metaimage::distanceCallback),
		  gensym("distance"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_metaimage::cheapCallback),
		  gensym("cheap"), A_DEFFLOAT, A_NULL);
}
void pix_metaimage :: sizeCallback(void *data, t_floatarg sz)
{
  GetMyClass(data)->m_Size=(sz);
  GetMyClass(data)->setPixModified();
}
void pix_metaimage :: distanceCallback(void *data, t_floatarg m_DoDistanceBased)
{
  GetMyClass(data)->m_DoDistanceBased=(m_DoDistanceBased);
  GetMyClass(data)->setPixModified();
}
void pix_metaimage :: cheapCallback(void *data, t_floatarg m_DoCheapAndNasty)
{
  GetMyClass(data)->m_DoCheapAndNasty=(m_DoCheapAndNasty);  
  GetMyClass(data)->setPixModified();
}
