////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// ported by tigital@mac.com
//	from "Pete's_Plugins"
//
// Implementation file
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_halftone.h"

#ifndef NDEBUG
# define NDEBUG
#endif
#include <assert.h>

CPPEXTERN_NEW(pix_halftone);

/////////////////////////////////////////////////////////
//
// pix_halftone
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_halftone :: pix_halftone()
{ 
    m_CellSize = 8; // 1-32
    m_Style = 0;	// 0-4
    m_Smoothing = 128; // 0-255
    m_Angle = 0.0f; // 0-360
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("size"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("angleDEG"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("smoothN"));
      
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_halftone :: ~pix_halftone()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_halftone :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    
    pSource = reinterpret_cast<U32*>(image.data);

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);
    
    int nCellSize=clampFunc(m_CellSize,1,nMaxCellSize);
    int nStyle=clampFunc(m_Style,0,4);
    int nSmoothingThreshold=clampFunc(m_Smoothing,0,255);

    const float AngleRadians=m_Angle;
    const int nCellSizeFP=(nCellSize<<nFPShift);

    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    unsigned char* pDotFuncTableStart=&g_pDotFuncTable[0];

    Pete_HalfTone_MakeDotFuncTable(pDotFuncTableStart,nCellSize,nStyle, 255.0f);

    unsigned char* pGreyScaleTableStart=&g_pGreyScaleTable[0];

    Pete_HalfTone_MakeGreyScaleTable(pGreyScaleTableStart,nSmoothingThreshold);

    SPete_HalfTone_Point Left;
    SPete_HalfTone_Point Right;
    SPete_HalfTone_Point Top;
    SPete_HalfTone_Point Bottom;
    Pete_HalfTone_CalcCorners(nWidth,nHeight,AngleRadians,nCellSize,&Left,&Right,&Top,&Bottom);

    //U32 TestColour=0x00ffffff;
    int nCurrentV;
    for (nCurrentV=Bottom.nY; nCurrentV<Top.nY; nCurrentV+=nCellSizeFP) {
      int nSnappedV=((nCurrentV+(nCellSizeFP*1024))/nCellSize);
      nSnappedV>>=nFPShift;
      nSnappedV<<=nFPShift;
      nSnappedV*=nCellSize;
      nSnappedV-=(nCellSizeFP*1024);

      int nLeftU;
      int nRightU;
      Pete_HalfTone_CalcSpanEnds(&Left,&Right,&Top,&Bottom,nSnappedV,&nLeftU,&nRightU);

      int nCurrentU;
      for (nCurrentU=nLeftU; nCurrentU<nRightU; nCurrentU+=nCellSizeFP) {
	int nSnappedU=(((nCurrentU+(nCellSizeFP*1024))/nCellSize));
	nSnappedU>>=nFPShift;
	nSnappedU<<=nFPShift;
	nSnappedU*=nCellSize;
	nSnappedU-=(nCellSizeFP*1024);
			
	SPete_HalfTone_Vertex RotatedPoints[4]={
	  {{nSnappedU,nSnappedV},				{0,0}},
	  {{(nSnappedU+nCellSizeFP),nSnappedV},			{nCellSizeFP-nFPMult,0}},
	  {{(nSnappedU+nCellSizeFP),(nSnappedV+nCellSizeFP)},	{nCellSizeFP-nFPMult,nCellSizeFP-nFPMult}},
	  {{nSnappedU,(nSnappedV+nCellSizeFP)},			{0,nCellSizeFP-nFPMult}}
	};

	SPete_HalfTone_Vertex ScreenSpacePoints[4];
	Pete_HalfTone_RotateMultipleVertices(
				&RotatedPoints[0],
				&ScreenSpacePoints[0],
				4,
				AngleRadians);

	int nCount;
	for (nCount=0; nCount<4; nCount+=1) {
	  ScreenSpacePoints[nCount].Pos.nX+=(nHalfWidth<<nFPShift);
	  ScreenSpacePoints[nCount].Pos.nY+=(nHalfHeight<<nFPShift);
	  ScreenSpacePoints[nCount].Pos.nX&=0xffff0000;
	  ScreenSpacePoints[nCount].Pos.nY&=0xffff0000;
	}

	SPete_HalfTone_Vertex CellLeft;
	SPete_HalfTone_Vertex CellRight;
	SPete_HalfTone_Vertex CellTop;
	SPete_HalfTone_Vertex CellBottom;
	Pete_HalfTone_GetRasterizationVertices(
				&ScreenSpacePoints[0],
				&CellLeft,&CellRight,&CellTop,&CellBottom);

	const int nSampleTopY=(ScreenSpacePoints[0].Pos.nY>>nFPShift);
	//const int nSampleBottomY=(nSampleTopY+nCellSize);
	//const int nSampleCenterY=(nSampleTopY+nHalfCellSize);
	const int nSampleLeftX=(ScreenSpacePoints[0].Pos.nX>>nFPShift);
	//const int nSampleRightX=(nSampleLeftX+nCellSize);
	//const int nSampleCenterX=(nSampleLeftX+nHalfCellSize);

	const U32 AverageColour=
	  Pete_GetImageAreaAverage(
				   nSampleLeftX,nSampleTopY,
				   nCellSize,nCellSize,
				   pSource,nWidth,nHeight);

	int nLuminance=GetLuminance(AverageColour)/256;
	nLuminance+=256;

	int nCurrentYFP;
	for (nCurrentYFP=CellBottom.Pos.nY; nCurrentYFP<=CellTop.Pos.nY; nCurrentYFP+=nFPMult) {
	  if (nCurrentYFP<0)  continue;

	  if (nCurrentYFP>=(nHeight<<nFPShift))	break;

	  SPete_HalfTone_Vertex SpanStart;
	  SPete_HalfTone_Vertex SpanEnd;
	  Pete_HalfTone_CalcSpanEnds_Vertex(
					&CellLeft,&CellRight,&CellTop,&CellBottom,nCurrentYFP,
					&SpanStart,&SpanEnd);

	  const int nCellLeftX=(SpanStart.Pos.nX>>nFPShift);
	  const int nCellRightX=(SpanEnd.Pos.nX>>nFPShift);
	  const int nCurrentY=(nCurrentYFP>>nFPShift);

	  int nLengthX=(nCellRightX-nCellLeftX);
	  if (nLengthX<1) nLengthX=1;

	  const int nGradientU=
	    (SpanEnd.TexCoords.nX-SpanStart.TexCoords.nX)/nLengthX;
	  const int nGradientV=
	    (SpanEnd.TexCoords.nY-SpanStart.TexCoords.nY)/nLengthX;

	  int nTexU=SpanStart.TexCoords.nX;
	  int nTexV=SpanStart.TexCoords.nY;

	  int nCurrentX;
	  for (nCurrentX=nCellLeftX; nCurrentX<nCellRightX;
	       nCurrentX+=1,nTexU+=nGradientU,nTexV+=nGradientV) {

	    if (nCurrentX<0)continue;
	    if (nCurrentX>=nWidth)break;

	    const int nTexUInt=(nTexU>>nFPShift);
	    const int nTexVInt=(nTexV>>nFPShift);

	    unsigned char* pCurrentDotFunc=
	      pDotFuncTableStart+
	      (nTexVInt*nCellSize)+
	      nTexUInt;

	    const int nDotFuncResult=*pCurrentDotFunc;
	    const int nDiff=nLuminance-nDotFuncResult;
	    const int nGreyValue=pGreyScaleTableStart[nDiff];
	    const int nAlphaValue=0xff;

	    const U32 OutputColour=
	      (nGreyValue<<SHIFT_RED)|
	      (nGreyValue<<SHIFT_GREEN)|
	      (nGreyValue<<SHIFT_BLUE)|
	      (nAlphaValue<<SHIFT_ALPHA);

	    U32* pCurrentOutput=
	      pOutput+(nCurrentY*nWidth)+nCurrentX;
	    *pCurrentOutput=OutputColour;

	  }
	}		
      }
    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_halftone :: processYUVImage(imageStruct &image)
{
    nWidth = image.xsize>>1;
    nHeight = image.ysize;
	
    const unsigned char chroma = 128;
    pSource = reinterpret_cast<U32*>(image.data);

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.csize = image.csize;
    myImage.format = image.format;
    myImage.type = image.type;
  //  myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);
    
    int nCellSize=clampFunc(m_CellSize,1,nMaxCellSize);
    int nStyle=clampFunc(m_Style,0,4);
    int nSmoothingThreshold=clampFunc(m_Smoothing,0,255);

    const float AngleRadians=m_Angle;
    const int nCellSizeFP=(nCellSize<<nFPShift);

    const int nHalfWidth=(nWidth>>1);
    const int nHalfHeight=(nHeight>>1);

    unsigned char* pDotFuncTableStart=&g_pDotFuncTable[0];

    Pete_HalfTone_MakeDotFuncTable(pDotFuncTableStart,nCellSize,nStyle, 235.0f);

    unsigned char* pGreyScaleTableStart=&g_pGreyScaleTable[0];

    //Pete_HalfTone_MakeGreyScaleTable(pGreyScaleTableStart,nSmoothingThreshold);
	YUV_MakeGreyScaleTable(pGreyScaleTableStart,nSmoothingThreshold);

    SPete_HalfTone_Point Left;
    SPete_HalfTone_Point Right;
    SPete_HalfTone_Point Top;
    SPete_HalfTone_Point Bottom;
    Pete_HalfTone_CalcCorners(nWidth,nHeight,AngleRadians,nCellSize,&Left,&Right,&Top,&Bottom);

    int nCurrentV;
    for (nCurrentV=Bottom.nY; nCurrentV<Top.nY; nCurrentV+=nCellSizeFP) {
      int nSnappedV=((nCurrentV+(nCellSizeFP*1024))/nCellSize);
      nSnappedV>>=nFPShift;
      nSnappedV<<=nFPShift;
      nSnappedV*=nCellSize;
      nSnappedV-=(nCellSizeFP*1024);

      int nLeftU;
      int nRightU;
      Pete_HalfTone_CalcSpanEnds(&Left,&Right,&Top,&Bottom,nSnappedV,&nLeftU,&nRightU);

      int nCurrentU;
      for (nCurrentU=nLeftU; nCurrentU<nRightU; nCurrentU+=nCellSizeFP) {
	    int nSnappedU=(((nCurrentU+(nCellSizeFP*1024))/nCellSize));
		nSnappedU>>=nFPShift;
		nSnappedU<<=nFPShift;
		nSnappedU*=nCellSize;
		nSnappedU-=(nCellSizeFP*1024);
			
		SPete_HalfTone_Vertex RotatedPoints[4]={ 
					{{nSnappedU,nSnappedV},				{0,0}},
					{{(nSnappedU+nCellSizeFP),nSnappedV},			{nCellSizeFP-nFPMult,0}},
					{{(nSnappedU+nCellSizeFP),(nSnappedV+nCellSizeFP)},	{nCellSizeFP-nFPMult,nCellSizeFP-nFPMult}},
					{{nSnappedU,(nSnappedV+nCellSizeFP)},			{0,nCellSizeFP-nFPMult}}
		};

		SPete_HalfTone_Vertex ScreenSpacePoints[4];
		Pete_HalfTone_RotateMultipleVertices(
				&RotatedPoints[0],
				&ScreenSpacePoints[0],
				4,
				AngleRadians);

		int nCount;
		for (nCount=0; nCount<4; nCount+=1) {
			ScreenSpacePoints[nCount].Pos.nX+=(nHalfWidth<<nFPShift);
			ScreenSpacePoints[nCount].Pos.nY+=(nHalfHeight<<nFPShift);
			ScreenSpacePoints[nCount].Pos.nX&=0xffff0000;
			ScreenSpacePoints[nCount].Pos.nY&=0xffff0000;
		}

		SPete_HalfTone_Vertex CellLeft;
		SPete_HalfTone_Vertex CellRight;
		SPete_HalfTone_Vertex CellTop;
		SPete_HalfTone_Vertex CellBottom;
		Pete_HalfTone_GetRasterizationVertices(
				&ScreenSpacePoints[0],
				&CellLeft,&CellRight,&CellTop,&CellBottom);

		const int nSampleTopY=(ScreenSpacePoints[0].Pos.nY>>nFPShift);
		const int nSampleLeftX=(ScreenSpacePoints[0].Pos.nX>>nFPShift);

		U32 nLuminance=
			GetImageAreaAverageLuma(
				   nSampleLeftX,nSampleTopY,
				   nCellSize,nCellSize,
				   pSource,nWidth,nHeight);
		nLuminance+=220;
		//nLuminance+=256;

		int nCurrentYFP;
		for (nCurrentYFP=CellBottom.Pos.nY; nCurrentYFP<=CellTop.Pos.nY; nCurrentYFP+=nFPMult) {
			if (nCurrentYFP<0)  continue;

			if (nCurrentYFP>=(nHeight<<nFPShift))	break;

			SPete_HalfTone_Vertex SpanStart;
			SPete_HalfTone_Vertex SpanEnd;
			Pete_HalfTone_CalcSpanEnds_Vertex(
					&CellLeft,&CellRight,&CellTop,&CellBottom,nCurrentYFP,
					&SpanStart,&SpanEnd);

			const int nCellLeftX=(SpanStart.Pos.nX>>nFPShift);
			const int nCellRightX=(SpanEnd.Pos.nX>>nFPShift);
			const int nCurrentY=(nCurrentYFP>>nFPShift);

			int nLengthX=(nCellRightX-nCellLeftX);
			if (nLengthX<1) nLengthX=1;

			const int nGradientU = (SpanEnd.TexCoords.nX-SpanStart.TexCoords.nX)/nLengthX;
			const int nGradientV = (SpanEnd.TexCoords.nY-SpanStart.TexCoords.nY)/nLengthX;

			int nTexU=SpanStart.TexCoords.nX;
			int nTexV=SpanStart.TexCoords.nY;

			int nCurrentX;
			for (nCurrentX=nCellLeftX; nCurrentX<nCellRightX;
					nCurrentX+=1,nTexU+=nGradientU,nTexV+=nGradientV) {

				if (nCurrentX<0)continue;
				if (nCurrentX>=nWidth)break;
				
				int nTexUInt=(nTexU>>nFPShift);
				int nTexVInt=(nTexV>>nFPShift);

				unsigned char* pCurrentDotFunc = pDotFuncTableStart+(nTexVInt*nCellSize)+nTexUInt;

				int nDotFuncResult=*pCurrentDotFunc;
				const int nDiff = nLuminance - nDotFuncResult;
				//const int nDiff = nLuma1 - nDotFuncResult;
				const int nGreyValue=pGreyScaleTableStart[nDiff];
				/*
				if (nDiff>383)
					diffHi += 1;
				if (nDiff<257)
					diffLo += 1;
				*/
				nTexV += nGradientV;
				nTexU += nGradientU;
				nTexVInt = (nTexV>>nFPShift);
				nTexUInt = (nTexU>>nFPShift);
				pCurrentDotFunc = pDotFuncTableStart+(nTexVInt*nCellSize)+nTexUInt;
				nDotFuncResult=*pCurrentDotFunc;
				const int nDiff2 = nLuminance - nDotFuncResult;
				//const int nDiff2 = nLuma2 - nDotFuncResult;
				const int nGreyValue2=pGreyScaleTableStart[nDiff2];
				//if( nGreyValue2 != nGreyValue )
				//	post("nGreyValue's !equal");
				/*
				if (nDiff2>383)
					diffHi += 1;
				if (nDiff2<257)
					diffLo += 1;
				*/
				const U32 OutputColour =    ((chroma&0xff)<<SHIFT_U)|
				  ((nGreyValue&0xff)<<SHIFT_Y1)|
				  ((chroma&0xff)<<SHIFT_V)|
				  ((nGreyValue2&0xff)<<SHIFT_Y2);

				U32* pCurrentOutput = pOutput+(nCurrentY*nWidth)+nCurrentX;
				*pCurrentOutput=OutputColour;
				}
			}
		}
    }
	//post("luma1cnt != 256: %d",luma1cnt);
	//post("luma2cnt != 256: %d",luma2cnt);
	//post(" diffHi = %d     diffLo = %d",diffHi, diffLo);
	//post("diff2Hi = %d    diff2Lo = %d",diff2Hi, diff2Lo);
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processGrayImage
//
/////////////////////////////////////////////////////////
void pix_halftone :: processGrayImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    
    if (!init) {
	Init(nWidth, nHeight);
	init = 1;
    }
    unsigned char*pSource = image.data;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    unsigned char* pOutput = myImage.data;
    
    int nCellSize=clampFunc(m_CellSize,1,nMaxCellSize);
    int nStyle=clampFunc(m_Style,0,4);
    int nSmoothingThreshold=clampFunc(m_Smoothing,0,255);

    const float AngleRadians=m_Angle;
    const int nCellSizeFP=(nCellSize<<nFPShift);

    const int nHalfWidth=(nWidth>>1);
    const int nHalfHeight=(nHeight>>1);

    unsigned char* pDotFuncTableStart=&g_pDotFuncTable[0];

    Pete_HalfTone_MakeDotFuncTable(pDotFuncTableStart,nCellSize,nStyle, 255.0f);

    unsigned char* pGreyScaleTableStart=&g_pGreyScaleTable[0];

    Pete_HalfTone_MakeGreyScaleTable(pGreyScaleTableStart,nSmoothingThreshold);

    SPete_HalfTone_Point Left;
    SPete_HalfTone_Point Right;
    SPete_HalfTone_Point Top;
    SPete_HalfTone_Point Bottom;
    Pete_HalfTone_CalcCorners(nWidth,nHeight,AngleRadians,nCellSize,&Left,&Right,&Top,&Bottom);

    int nCurrentV;
    for (nCurrentV=Bottom.nY; nCurrentV<Top.nY; nCurrentV+=nCellSizeFP) {
      int nSnappedV=((nCurrentV+(nCellSizeFP*1024))/nCellSize);
      nSnappedV>>=nFPShift;
      nSnappedV<<=nFPShift;
      nSnappedV*=nCellSize;
      nSnappedV-=(nCellSizeFP*1024);

      int nLeftU;
      int nRightU;
      Pete_HalfTone_CalcSpanEnds(&Left,&Right,&Top,&Bottom,nSnappedV,&nLeftU,&nRightU);

      int nCurrentU;
      for (nCurrentU=nLeftU; nCurrentU<nRightU; nCurrentU+=nCellSizeFP) {
	int nSnappedU=(((nCurrentU+(nCellSizeFP*1024))/nCellSize));
	nSnappedU>>=nFPShift;
	nSnappedU<<=nFPShift;
	nSnappedU*=nCellSize;
	nSnappedU-=(nCellSizeFP*1024);
			
	SPete_HalfTone_Vertex RotatedPoints[4]={
	  {{nSnappedU,nSnappedV},				{0,0}},
	  {{(nSnappedU+nCellSizeFP),nSnappedV},			{nCellSizeFP-nFPMult,0}},
	  {{(nSnappedU+nCellSizeFP),(nSnappedV+nCellSizeFP)},	{nCellSizeFP-nFPMult,nCellSizeFP-nFPMult}},
	  {{nSnappedU,(nSnappedV+nCellSizeFP)},			{0,nCellSizeFP-nFPMult}}
	};

	SPete_HalfTone_Vertex ScreenSpacePoints[4];
	Pete_HalfTone_RotateMultipleVertices(
				&RotatedPoints[0],
				&ScreenSpacePoints[0],
				4,
				AngleRadians);

	int nCount;
	for (nCount=0; nCount<4; nCount+=1) {
	  ScreenSpacePoints[nCount].Pos.nX+=(nHalfWidth<<nFPShift);
	  ScreenSpacePoints[nCount].Pos.nY+=(nHalfHeight<<nFPShift);
	  ScreenSpacePoints[nCount].Pos.nX&=0xffff0000;
	  ScreenSpacePoints[nCount].Pos.nY&=0xffff0000;
	}

	SPete_HalfTone_Vertex CellLeft;
	SPete_HalfTone_Vertex CellRight;
	SPete_HalfTone_Vertex CellTop;
	SPete_HalfTone_Vertex CellBottom;
	Pete_HalfTone_GetRasterizationVertices(
				&ScreenSpacePoints[0],
				&CellLeft,&CellRight,&CellTop,&CellBottom);

	const int nSampleTopY=(ScreenSpacePoints[0].Pos.nY>>nFPShift);
	const int nSampleLeftX=(ScreenSpacePoints[0].Pos.nX>>nFPShift);
	int nLuminance =
	  Pete_GetImageAreaAverageGray(
				   nSampleLeftX,nSampleTopY,
				   nCellSize,nCellSize,
				   pSource,nWidth,nHeight);
	nLuminance+=256;

	int nCurrentYFP;
	for (nCurrentYFP=CellBottom.Pos.nY; nCurrentYFP<=CellTop.Pos.nY; nCurrentYFP+=nFPMult) {
	  if (nCurrentYFP<0)  continue;

	  if (nCurrentYFP>=(nHeight<<nFPShift))	break;

	  SPete_HalfTone_Vertex SpanStart;
	  SPete_HalfTone_Vertex SpanEnd;
	  Pete_HalfTone_CalcSpanEnds_Vertex(
					&CellLeft,&CellRight,&CellTop,&CellBottom,nCurrentYFP,
					&SpanStart,&SpanEnd);

	  const int nCellLeftX=(SpanStart.Pos.nX>>nFPShift);
	  const int nCellRightX=(SpanEnd.Pos.nX>>nFPShift);
	  const int nCurrentY=(nCurrentYFP>>nFPShift);

	  int nLengthX=(nCellRightX-nCellLeftX);
	  if (nLengthX<1) nLengthX=1;

	  const int nGradientU=
	    (SpanEnd.TexCoords.nX-SpanStart.TexCoords.nX)/nLengthX;
	  const int nGradientV=
	    (SpanEnd.TexCoords.nY-SpanStart.TexCoords.nY)/nLengthX;

	  int nTexU=SpanStart.TexCoords.nX;
	  int nTexV=SpanStart.TexCoords.nY;

	  int nCurrentX;
	  for (nCurrentX=nCellLeftX; nCurrentX<nCellRightX;
	       nCurrentX+=1,nTexU+=nGradientU,nTexV+=nGradientV) {

	    if (nCurrentX<0)continue;
	    if (nCurrentX>=nWidth)break;

	    const int nTexUInt=(nTexU>>nFPShift);
	    const int nTexVInt=(nTexV>>nFPShift);

	    unsigned char* pCurrentDotFunc=
	      pDotFuncTableStart+
	      (nTexVInt*nCellSize)+
	      nTexUInt;
              
	    unsigned char* pCurrentOutput=
	      pOutput+(nCurrentY*nWidth)+nCurrentX;
	    *pCurrentOutput=pGreyScaleTableStart[nLuminance-*pCurrentDotFunc];
	  }
	}
      }
    }
    image.data = myImage.data;
}
/////////////////////////////////////////////////////////
// various processing here
//
/////////////////////////////////////////////////////////
int pix_halftone :: Init(int nWidth, int nHeight)
{
	//Pete_HalfTone_DeInit();

	//pInstanceData->nWidth=nWidth;
	//pInstanceData->nHeight=nHeight;

	return 1;
}

void pix_halftone :: Pete_HalfTone_DeInit() {
  // do nothing
}
int pix_halftone :: RoundDotFunc(float X,float Y, float scale) {
  const float XSquared=(X*X);
  const float YSquared=(Y*Y);
  const float Result=(2.0f-(XSquared+YSquared))/2.0f;

  return static_cast<int>(Result*scale);
}

int pix_halftone :: LineDotFunc(float X,float Y, float scale) {
  const float Result=(1.0f-fabsf(Y));
  return static_cast<int>(Result*scale);
}

int pix_halftone :: DiamondDotFunc(float X,float Y, float scale) {
  const float Result=(2.0f-(fabsf(X)+fabsf(Y)))/2.0f;
  return static_cast<int>(Result*scale);
}

int pix_halftone :: EuclideanDotFunc(float X,float Y, float scale) {
  const float AbsX=fabsf(X);
  const float AbsY=fabsf(Y);
  float Result;

  if ((AbsX+AbsY)>1.0f) {
    Result=((AbsY-1.0f)*(AbsY-1.0f)+
	    (AbsX-1.0f)*(AbsX-1.0f))-2.0f;
  } else {
    Result=2.0f-(AbsY*AbsY+AbsX*AbsX);
  }

  Result/=2.0f;

  return static_cast<int>(Result*scale);
}

int pix_halftone :: PSDiamondDotFunc(float X,float Y, float scale) {
  const float AbsX=fabsf(X);
  const float AbsY=fabsf(Y);
	
  float Result;

  if ((AbsX+AbsY)<=1.5f) {
    Result=2.0f-(AbsX*AbsX+AbsY*AbsY); 
  } else if ((AbsX+AbsY)<=1.23f) { 
    Result=2.0f-((AbsY*0.76f)+AbsX);
  } else {
    Result=((AbsY-1.0f)*(AbsY-1.0f)+
	    (AbsX-1.0f)*(AbsX-1.0f))-2.0f;	
  }

  Result/=2.0f;
  return static_cast<int>(Result*scale);
}

void pix_halftone :: Rotate(SPete_HalfTone_Point* pinPoint,SPete_HalfTone_Point* poutPoint,float Angle)
{
  const int CosFP=static_cast<int>(cos(Angle)*nFPMult);
  const int SinFP=static_cast<int>(sin(Angle)*nFPMult);

  poutPoint->nX=
    (CosFP*(pinPoint->nX>>nFPShift))+
    (SinFP*(pinPoint->nY>>nFPShift));

  poutPoint->nY=
    (CosFP*(pinPoint->nY>>nFPShift))-
    (SinFP*(pinPoint->nX>>nFPShift));
}

void pix_halftone :: Pete_HalfTone_MakeDotFuncTable(unsigned char* pDotFuncTableStart,int nCellSize,int nStyle,
													float scale) {
  const int nHalfCellSize=(nCellSize/2);
  unsigned char* pCurrentDotFunc=pDotFuncTableStart;
  int nCurrentY;
  for (nCurrentY=0; nCurrentY<nCellSize; nCurrentY+=1) {
    float NormalY=(nCurrentY-nHalfCellSize)/(nCellSize*0.5f);
    int nCurrentX;
    for (nCurrentX=0; nCurrentX<nCellSize; nCurrentX+=1,pCurrentDotFunc+=1) {
      float NormalX=(nCurrentX-nHalfCellSize)/(nCellSize*0.5f);
      int nDotFuncResult;
      switch (nStyle) {
      case eRoundStyle: {
	nDotFuncResult= RoundDotFunc(NormalX,NormalY, scale);
      }break;
      case eLineStyle: {
	nDotFuncResult=LineDotFunc(NormalX,NormalY, scale);
      }break;
      case eDiamondStyle: {
	nDotFuncResult=DiamondDotFunc(NormalX,NormalY, scale);
      }break;
      case eEuclideanStyle: {
	nDotFuncResult=EuclideanDotFunc(NormalX,NormalY, scale);
      }break;
      case ePSDiamond: {
	nDotFuncResult=PSDiamondDotFunc(NormalX,NormalY, scale);
      }break;
      default: {
	assert(false);
	nDotFuncResult=static_cast<int>(scale);
      }break;
      }
      *pCurrentDotFunc=nDotFuncResult;
    }
  }
}

void pix_halftone :: Pete_HalfTone_CalcCorners(int cWidth,int nHeight,float AngleRadians,int nCellSize,
	SPete_HalfTone_Point* poutLeft,
	SPete_HalfTone_Point* poutRight,
	SPete_HalfTone_Point* poutTop,
	SPete_HalfTone_Point* poutBottom) {

  const int nHalfWidth=(cWidth/2);
  const int nHalfHeight=(nHeight/2);
  const int nHalfWidthFP=(nHalfWidth<<nFPShift);
  const int nHalfHeightFP=(nHalfHeight<<nFPShift);

  const int nCellSizeFP=(nCellSize<<nFPShift);
  const int nXExtentFP=nHalfWidthFP+(2*nCellSizeFP);
  const int nYExtentFP=nHalfHeightFP+(2*nCellSizeFP);

  SPete_HalfTone_Point OriginalCorners[4]={
    {-nXExtentFP,-nYExtentFP},
    {nXExtentFP,-nYExtentFP},
    {-nXExtentFP,nYExtentFP},
    {nXExtentFP,nYExtentFP},
  };
  SPete_HalfTone_Point RotatedCorners[4];

  int nCount;
  for (nCount=0; nCount<4; nCount+=1) {
    Rotate(	&OriginalCorners[nCount],
		&RotatedCorners[nCount],
		-AngleRadians);
  }

  GetRasterizationPoints(
			 &RotatedCorners[0],
			 poutLeft,
			 poutRight,
			 poutTop,
			 poutBottom);
}

void pix_halftone :: GetRasterizationPoints(
	SPete_HalfTone_Point* pinPoints,
	SPete_HalfTone_Point* poutLeft,
	SPete_HalfTone_Point* poutRight,
	SPete_HalfTone_Point* poutTop,
	SPete_HalfTone_Point* poutBottom) {
  HeightSortPoints(pinPoints,4);
  *poutBottom=pinPoints[0];
  *poutTop=pinPoints[3];

  if (pinPoints[1].nX<pinPoints[2].nX) {
    *poutLeft=pinPoints[1];
    *poutRight=pinPoints[2];
  } else {
    *poutLeft=pinPoints[2];
    *poutRight=pinPoints[1];
  }
}

void pix_halftone :: HeightSortPoints(SPete_HalfTone_Point* pPoints,int nPointCount) {
  int nCount;
  for (nCount=0; nCount<nPointCount; nCount+=1) {
    int nLowestIndex=nCount;
    int nIndex;
    for (nIndex=(nCount+1); nIndex<nPointCount; nIndex+=1) {
      if (pPoints[nIndex].nY< pPoints[nLowestIndex].nY) {
	nLowestIndex=nIndex;
      } else if (pPoints[nIndex].nY==pPoints[nLowestIndex].nY) {
	if (pPoints[nIndex].nX<pPoints[nLowestIndex].nX) {
	  nLowestIndex=nIndex;
	}
      }
    }

    SPete_HalfTone_Point SwapTemp;
    SwapTemp=pPoints[nCount];
    pPoints[nCount]=pPoints[nLowestIndex];
    pPoints[nLowestIndex]=SwapTemp;
  }
}

void pix_halftone :: Pete_HalfTone_CalcSpanEnds(
	SPete_HalfTone_Point* pinLeft,
	SPete_HalfTone_Point* pinRight,
	SPete_HalfTone_Point* pinTop,
	SPete_HalfTone_Point* pinBottom,
	int nY,int* poutLeftX,int* poutRightX) {
  LerpAlongEdges(pinBottom,pinLeft,pinTop,nY,poutLeftX);
  LerpAlongEdges(pinBottom,pinRight,pinTop,nY,poutRightX);
}

void pix_halftone :: LerpAlongEdges(
	SPete_HalfTone_Point* pStart,
	SPete_HalfTone_Point* pMiddle,
	SPete_HalfTone_Point* pEnd,
	int nY,int* poutX) {
  if (nY<pMiddle->nY) {
    const int nYDist=pMiddle->nY-pStart->nY;
    if (nYDist<nFPMult) *poutX=pStart->nX;
    else {
      const int nOneMinusLerpValue=
	(nY-pStart->nY)/(nYDist>>nFPShift);
      const int nLerpValue=nFPMult-nOneMinusLerpValue;
      *poutX=
	(nLerpValue*(pStart->nX>>nFPShift))+
	(nOneMinusLerpValue*(pMiddle->nX>>nFPShift));
    }
  } else {
    const int nYDist=pEnd->nY-pMiddle->nY;
    if (nYDist<nFPMult) *poutX=pMiddle->nX;
    else {
      const int nOneMinusLerpValue=
	(nY-pMiddle->nY)/(nYDist>>nFPShift);
      const int nLerpValue=nFPMult-nOneMinusLerpValue;
      *poutX=
	(nLerpValue*(pMiddle->nX>>nFPShift))+
	(nOneMinusLerpValue*(pEnd->nX>>nFPShift));
    }
  }
}

void pix_halftone :: RotateMultiple(SPete_HalfTone_Point* pinPoints,
	SPete_HalfTone_Point* poutPoints,
	int nPointCount,
	float Angle) {

  const int CosFP=static_cast<int>(cos(Angle)*nFPMult);
  const int SinFP=static_cast<int>(sin(Angle)*nFPMult);

  int nCount;
  for (nCount=0; nCount<nPointCount; nCount+=1) {
    poutPoints[nCount].nX=
      (CosFP*(pinPoints[nCount].nX>>nFPShift))+
      (SinFP*(pinPoints[nCount].nY>>nFPShift));

    poutPoints[nCount].nY=
      (CosFP*(pinPoints[nCount].nY>>nFPShift))-
      (SinFP*(pinPoints[nCount].nX>>nFPShift));
  }
}

void pix_halftone :: Pete_HalfTone_CalcSpanEnds_Vertex(
	SPete_HalfTone_Vertex* pinLeft,
	SPete_HalfTone_Vertex* pinRight,
	SPete_HalfTone_Vertex* pinTop,
	SPete_HalfTone_Vertex* pinBottom,
	int nY,
	SPete_HalfTone_Vertex* poutLeft,
	SPete_HalfTone_Vertex* poutRight) {

  Pete_HalfTone_LerpAlongEdges_Vertex(pinBottom,pinLeft,pinTop,nY,poutLeft);
  Pete_HalfTone_LerpAlongEdges_Vertex(pinBottom,pinRight,pinTop,nY,poutRight);
}

void pix_halftone :: Pete_HalfTone_LerpAlongEdges_Vertex(
	SPete_HalfTone_Vertex* pStart,
	SPete_HalfTone_Vertex* pMiddle,
	SPete_HalfTone_Vertex* pEnd,
	int nY,
	SPete_HalfTone_Vertex* poutVertex) {
  const int nMiddleY=pMiddle->Pos.nY;
  if (nY<nMiddleY) {
    const int nStartX=pStart->Pos.nX;
    const int nStartY=pStart->Pos.nY;
    const int nStartU=pStart->TexCoords.nX;
    const int nStartV=pStart->TexCoords.nY;
    const int nYDist=nMiddleY-nStartY;

    if (nYDist<nFPMult) {
      poutVertex->Pos.nX=nStartX;
      poutVertex->TexCoords.nX=nStartU;
      poutVertex->TexCoords.nY=nStartV;
    } else {
     const int nMiddleX=pMiddle->Pos.nX;
      const int nMiddleU=pMiddle->TexCoords.nX;
      const int nMiddleV=pMiddle->TexCoords.nY;

      const int nOneMinusLerpValue=
	(nY-nStartY)/(nYDist>>nFPShift);
      const int nLerpValue=nFPMult-nOneMinusLerpValue;

      poutVertex->Pos.nX=
	(nLerpValue*(nStartX>>nFPShift))+
	(nOneMinusLerpValue*(nMiddleX>>nFPShift));

      poutVertex->TexCoords.nX=
	(nLerpValue*(nStartU>>nFPShift))+
	(nOneMinusLerpValue*(nMiddleU>>nFPShift));

      poutVertex->TexCoords.nY=
	(nLerpValue*(nStartV>>nFPShift))+
	(nOneMinusLerpValue*(nMiddleV>>nFPShift));
    }
  } else {
    const int nEndY=pEnd->Pos.nY;
    const int nYDist=nEndY-nMiddleY;
    const int nMiddleX=pMiddle->Pos.nX;
    const int nMiddleU=pMiddle->TexCoords.nX;
    const int nMiddleV=pMiddle->TexCoords.nY;

    if (nYDist<nFPMult) {
     poutVertex->Pos.nX=nMiddleX;
      poutVertex->TexCoords.nX=nMiddleU;
      poutVertex->TexCoords.nY=nMiddleV;
    } else {
     const int nEndX=pEnd->Pos.nX;
      const int nEndU=pEnd->TexCoords.nX;
      const int nEndV=pEnd->TexCoords.nY;

      const int nOneMinusLerpValue=
	(nY-nMiddleY)/(nYDist>>nFPShift);
      const int nLerpValue=nFPMult-nOneMinusLerpValue;
      
      poutVertex->Pos.nX=
	(nLerpValue*(nMiddleX>>nFPShift))+
	(nOneMinusLerpValue*(nEndX>>nFPShift));
      
      poutVertex->TexCoords.nX=
	(nLerpValue*(nMiddleU>>nFPShift))+
	(nOneMinusLerpValue*(nEndU>>nFPShift));

      poutVertex->TexCoords.nY=
	(nLerpValue*(nMiddleV>>nFPShift))+
	(nOneMinusLerpValue*(nEndV>>nFPShift));
    }
  }
  poutVertex->Pos.nY=nY;
}

void pix_halftone :: Pete_HalfTone_GetRasterizationVertices(
	SPete_HalfTone_Vertex* pinVertices,
	SPete_HalfTone_Vertex* poutLeft,
	SPete_HalfTone_Vertex* poutRight,
	SPete_HalfTone_Vertex* poutTop,
	SPete_HalfTone_Vertex* poutBottom) {

  const int nLowestVertex=Pete_HalfTone_GetLowestVertex(pinVertices,4);

  *poutBottom=pinVertices[(nLowestVertex+0)%4];
  *poutTop=pinVertices[(nLowestVertex+2)%4];

  //	if (pinVertices[1].Pos.nX<pinVertices[2].Pos.nX) {
  *poutLeft=pinVertices[(nLowestVertex+3)%4];
  *poutRight=pinVertices[(nLowestVertex+1)%4];
//	} else {
//		*poutLeft=pinVertices[2];
//		*poutRight=pinVertices[1];
//	}

//	int DirDot=
//		(poutTop->Pos.nX-poutLeft->Pos.nX)*
//		(poutBottom->Pos.nX-poutRight->Pos.nX);
//
//	
//	if ((poutLeft->Pos.nX==poutRight->Pos.nX)||(DirDot>0)) {
//
//		SPete_HalfTone_Vertex SwapTemp;
//
//		if (poutLeft->Pos.nX>poutTop->Pos.nX) {
//
//			SwapTemp=*poutLeft;
//			*poutLeft=*poutTop;
//			*poutTop=SwapTemp;
//
//		} else {
//
//			SwapTemp=*poutRight;
//			*poutRight=*poutBottom;
//			*poutBottom=SwapTemp;
//
//		}
//
//		poutTop->TexCoords.nX=0x00080000;
//		poutTop->TexCoords.nY=0x00080000;
//
//		poutBottom->TexCoords.nX=0x00080000;
//		poutBottom->TexCoords.nY=0x00080000;
//
//		poutLeft->TexCoords.nX=0x00080000;
//		poutLeft->TexCoords.nY=0x00080000;
//
//		poutRight->TexCoords.nX=0x00080000;
//		poutRight->TexCoords.nY=0x00080000;
//
//	}
}

int pix_halftone :: Pete_HalfTone_GetLowestVertex(SPete_HalfTone_Vertex* pVertices,int nVertexCount) {

  //	int nCount;
  //	for (nCount=0; nCount<nVertexCount; nCount+=1) {

  int nLowestIndex=0;

  int nIndex;
  for (nIndex=1; nIndex<nVertexCount; nIndex+=1) {

    if (pVertices[nIndex].Pos.nY<pVertices[nLowestIndex].Pos.nY) {
      nLowestIndex=nIndex;
    } else if (pVertices[nIndex].Pos.nY==pVertices[nLowestIndex].Pos.nY) {
      if (pVertices[nIndex].Pos.nX<pVertices[nLowestIndex].Pos.nX) {
	nLowestIndex=nIndex;
      }
    }
  }

//		SPete_HalfTone_Vertex SwapTemp;
//		SwapTemp=pVertices[nCount];
//		pVertices[nCount]=pVertices[nLowestIndex];
//		pVertices[nLowestIndex]=SwapTemp;
//
//	}
  return nLowestIndex;
}

void pix_halftone :: Pete_HalfTone_RotateMultipleVertices(SPete_HalfTone_Vertex* pinVertices,
	SPete_HalfTone_Vertex* poutVertices,
	int nVertexCount,
	float Angle) {

  const int CosFP=static_cast<int>(cos(Angle)*nFPMult);
  const int SinFP=static_cast<int>(sin(Angle)*nFPMult);

  int nCount;
  for (nCount=0; nCount<nVertexCount; nCount+=1) {
    poutVertices[nCount].Pos.nX=
      (CosFP*(pinVertices[nCount].Pos.nX>>nFPShift))+
      (SinFP*(pinVertices[nCount].Pos.nY>>nFPShift));

    poutVertices[nCount].Pos.nY=
      (CosFP*(pinVertices[nCount].Pos.nY>>nFPShift))-
      (SinFP*(pinVertices[nCount].Pos.nX>>nFPShift));

    poutVertices[nCount].TexCoords=pinVertices[nCount].TexCoords;
  }
}

void pix_halftone :: Pete_HalfTone_MakeGreyScaleTable(unsigned char* pGreyScaleTableStart,int nSmoothingThreshold) {

  if (nSmoothingThreshold<=0)  nSmoothingThreshold=1;

  int nCount;
  for (nCount=0; nCount<512; nCount+=1) {
    const int nDiff=nCount-256;
    int nGreyValue;
    if (nDiff<0) nGreyValue=0;
    else {
      if (nDiff>nSmoothingThreshold) nGreyValue=255;
      else nGreyValue=(nDiff*255)/nSmoothingThreshold;
    }
    
    pGreyScaleTableStart[nCount]=nGreyValue;
	//if (!init)
	//	post ("pGreyScaleTableStart[%d] = %d",nCount,nGreyValue);
  }
  init=1;
}

void pix_halftone :: YUV_MakeGreyScaleTable(unsigned char* pGreyScaleTableStart,int nSmoothingThreshold) {

  if (nSmoothingThreshold<=0)  nSmoothingThreshold=1;

  int nCount;
  for (nCount=0; nCount<470; nCount+=1) {
    const int nDiff=nCount-235;
    int nGreyValue;
    if (nDiff<16) nGreyValue=16;
    else {
      if (nDiff>nSmoothingThreshold) nGreyValue=235;
      else nGreyValue=(nDiff*235)/nSmoothingThreshold;
    }
    
    pGreyScaleTableStart[nCount]=nGreyValue;
	//if (!init)
	//	post ("pGreyScaleTableStart[%d] = %d",nCount,nGreyValue);
  }
  init=1;
}

U32 pix_halftone :: Pete_GetImageAreaAverage(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,U32* pImageData,int nImageWidth,int nImageHeight) {

  if (nLeftX<0) {
    nDeltaX-=(0-nLeftX);   nLeftX=0;
  }

  if (nTopY<0) {
    nDeltaY-=(0-nTopY);    nTopY=0;
  }

  if ((nLeftX+nDeltaX)>=nImageWidth) nDeltaX-=((nLeftX+nDeltaX)-(nImageWidth-1));

  if ((nTopY+nDeltaY)>=nImageHeight) nDeltaY-=((nTopY+nDeltaY)-(nImageHeight-1));

  if ((nDeltaX<1)||(nDeltaY<1)) return 0;

  U32* pSourceStart=    pImageData+(nTopY*nImageWidth)+nLeftX;
  U32* pSourceEnd=      pSourceStart+(nDeltaY*nImageWidth);
  U32* pCurrentSource=pSourceStart;
  
  int nRedTotal=0;
  int nGreenTotal=0;
  int nBlueTotal=0;
  int nAlphaTotal=0;

  while (pCurrentSource<pSourceEnd) {
    U32* pSourceLineStart=pCurrentSource;
    U32* pSourceLineEnd=pCurrentSource+nDeltaX;

    while (pCurrentSource<pSourceLineEnd) {
      const U32 CurrentColour=*pCurrentSource;
      const int nCurrentRed=(CurrentColour>>SHIFT_RED)&0xff;
      const int nCurrentGreen=(CurrentColour>>SHIFT_GREEN)&0xff;
      const int nCurrentBlue=(CurrentColour>>SHIFT_BLUE)&0xff;
      const int nCurrentAlpha=(CurrentColour>>SHIFT_ALPHA)&0xff;


      nRedTotal+=nCurrentRed;
      nGreenTotal+=nCurrentGreen;
      nBlueTotal+=nCurrentBlue;
      nAlphaTotal+=nCurrentAlpha;

      pCurrentSource+=1;
    }
    pCurrentSource=pSourceLineStart+nImageWidth;
  }
  const int nTotalSamples=(nDeltaX*nDeltaY);
  const int nRedAverage=(nRedTotal/nTotalSamples);
  const int nGreenAverage=(nGreenTotal/nTotalSamples);
  const int nBlueAverage=(nBlueTotal/nTotalSamples);
  const int nAlphaAverage=(nAlphaTotal/nTotalSamples);

  return (nRedAverage<<SHIFT_RED)|(nGreenAverage<<SHIFT_GREEN)|(nBlueAverage<<SHIFT_BLUE)|(nAlphaAverage<<SHIFT_ALPHA);
}

U32 pix_halftone :: GetImageAreaAverageLuma(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,U32* pImageData,int nImageWidth,int nImageHeight) {

  if (nLeftX<0) {
    nDeltaX-=(0-nLeftX);   nLeftX=0;
  }

  if (nTopY<0) {
    nDeltaY-=(0-nTopY);    nTopY=0;
  }

  if ((nLeftX+nDeltaX)>=(nImageWidth)) nDeltaX-=((nLeftX+nDeltaX)-((nImageWidth)-1));

  if ((nTopY+nDeltaY)>=nImageHeight) nDeltaY-=((nTopY+nDeltaY)-(nImageHeight-1));

  if ((nDeltaX<1)||(nDeltaY<1)) return 0;

  U32* pSourceStart=    pImageData+(nTopY*nImageWidth)+nLeftX;
  U32* pSourceEnd=      pSourceStart+(nDeltaY*nImageWidth);
  U32* pCurrentSource=pSourceStart;
  
  int nLumaTotal =0;
  int nLuma1, nLuma2 = 0;

  while (pCurrentSource<pSourceEnd) {
    U32* pSourceLineStart=pCurrentSource;
    U32* pSourceLineEnd=pCurrentSource+nDeltaX;

    while (pCurrentSource<pSourceLineEnd) {
		const U32 CurrentColour=*pCurrentSource;
		//nLuma1 = ((CurrentColour&(0xff<<16))>>16)<<8;
		//nLuma2 = ((CurrentColour&(0xff<<0))>>0)<<8;
		
		nLuma1 = ((CurrentColour&(0xff<<SHIFT_Y1))>>SHIFT_Y1);
		nLuma2 = ((CurrentColour&(0xff<<SHIFT_Y2))>>SHIFT_Y2);
		
		nLumaTotal += nLuma1 + nLuma2;
		//nLuma1Total += nLuma1;
		//nLuma2Total += nLuma2;
		//cnt+=1;

      pCurrentSource+=1;
    }
    pCurrentSource=pSourceLineStart+nImageWidth;
  }
 // post("loop # = %d",cnt);

  //const int nTotalSamples=(nDeltaX*nDeltaY);
  const int nTotalSamples=(nDeltaX*nDeltaY)*2;
  //post("nTotalSamples = %d",nTotalSamples);
  const int nLumaAverage=(nLumaTotal/nTotalSamples);
  //post("%d/%d=%d", nLumaTotal, nTotalSamples, nLumaAverage);
  return (nLumaAverage);
}

unsigned char pix_halftone :: Pete_GetImageAreaAverageGray(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,unsigned char* pImageData,int nImageWidth,int nImageHeight) {
  /* grey-scale images: jmz */
  if (nLeftX<0) {
    nDeltaX-=(0-nLeftX);   nLeftX=0;
  }

  if (nTopY<0) {
    nDeltaY-=(0-nTopY);    nTopY=0;
  }

  if ((nLeftX+nDeltaX)>=nImageWidth) nDeltaX-=((nLeftX+nDeltaX)-(nImageWidth-1));

  if ((nTopY+nDeltaY)>=nImageHeight) nDeltaY-=((nTopY+nDeltaY)-(nImageHeight-1));

  if ((nDeltaX<1)||(nDeltaY<1)) return 0;

  unsigned char* pSourceStart=    pImageData+(nTopY*nImageWidth)+nLeftX;
  unsigned char* pSourceEnd=      pSourceStart+(nDeltaY*nImageWidth);
  unsigned char* pCurrentSource=pSourceStart;
  
  int nGreyTotal=0;

  while (pCurrentSource<pSourceEnd) {
    unsigned char* pSourceLineStart=pCurrentSource;
    unsigned char* pSourceLineEnd=pCurrentSource+nDeltaX;

    while (pCurrentSource<pSourceLineEnd) nGreyTotal+=*pCurrentSource++;

    pCurrentSource=pSourceLineStart+nImageWidth;

  }

  const int nTotalSamples=(nDeltaX*nDeltaY);
  const int nGreyAverage=static_cast<int>(nGreyTotal/nTotalSamples);
  return nGreyAverage;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_halftone :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::sizeCallback),
		  gensym("size"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::styleCallback),
		  gensym("style"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::smoothCallback),
		  gensym("smooth"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::smoothNCallback),
		  gensym("smoothN"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::angleCallback),
		  gensym("angle"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_halftone::angleDEGCallback),
		  gensym("angleDEG"), A_DEFFLOAT, A_NULL);
}

void pix_halftone :: sizeCallback(void *data, t_floatarg m_CellSize)
{
  int size=static_cast<int>(m_CellSize);
  if(size<1){
    GetMyClass(data)->error("size must not be < 0");
    size=1;
  }
  if(size>nMaxCellSize){
    GetMyClass(data)->error("size must not be > %d", nMaxCellSize);
    size=nMaxCellSize;
  }
  GetMyClass(data)->m_CellSize=size;
  GetMyClass(data)->setPixModified();
}

void pix_halftone :: styleCallback(void *data, t_floatarg m_Style)
{
  int style=static_cast<int>(m_Style);
  if(style<0||style>4){
    GetMyClass(data)->error("style must be 0, 1, 2, 3 or 4");
    return;
  }
  GetMyClass(data)->m_Style=style;
  GetMyClass(data)->setPixModified();
}
void pix_halftone :: smoothCallback(void *data, t_floatarg m_Smoothing)
{
  GetMyClass(data)->m_Smoothing=CLAMP(m_Smoothing);  
  GetMyClass(data)->setPixModified();
}
void pix_halftone :: angleCallback(void *data, t_floatarg m_Angle)
{
  GetMyClass(data)->m_Angle=(m_Angle);  
  GetMyClass(data)->setPixModified();
}
void pix_halftone :: smoothNCallback(void *data, t_floatarg m_Smoothing)
{
  GetMyClass(data)->m_Smoothing=CLAMP(255.f*m_Smoothing);  
  GetMyClass(data)->setPixModified();
}
void pix_halftone :: angleDEGCallback(void *data, t_floatarg m_Angle)
{
  GetMyClass(data)->m_Angle=(atan2f(1,1)*m_Angle/45.0);  
  GetMyClass(data)->setPixModified();
}
