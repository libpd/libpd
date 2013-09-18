////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
// ported from pete's_plugins (www.petewarden.com)
//
// Implementation file
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_kaleidoscope.h"

static double deg2rad=atan2(1.,1.)/45.0;

CPPEXTERN_NEW(pix_kaleidoscope);

/////////////////////////////////////////////////////////
//
// pix_kaleidoscope
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_kaleidoscope :: pix_kaleidoscope()
{ 
  m_Divisions = 7.0f;
  m_OutputAnglePreIncrement = 0.0f;
  m_SourceAnglePreIncrement = 0.0f;
  m_SourceCentreX = 0.5f;
  m_SourceCentreY = 0.5f;
  m_OutputCentreX = 0.5f;
  m_OutputCentreY = 0.5f;
  m_ReflectionLineProportion = 0.5f;
  m_SourceAngleProportion = 1.0f;
  nMaxLines=128;
  nCosTableSizeShift = 10;
  nCosTableSize = (1<<nCosTableSizeShift);

  nFixedShift = 16;
  nFixedMult = (1<<nFixedShift);
  nFixedMask = (nFixedMult-1);

  nDistanceShift = 6;
  nDistanceMult = (1<<nDistanceShift);

  FixedAngleMult = (nFixedMult/Pete_TwoPi);

  nHalfPiFA = (nFixedMult/4);

  nFAToCosTableShift = (nFixedShift - nCosTableSizeShift);

  Pete_Kaleidoscope_Epsilon = 0.0001f;

  nMaxDivisions = 64;
  init =0;

  m_inDiv=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("div"));
  m_inSAngle=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("sourceAngle"));
  m_inSCtr  =inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("sourceCtr"));
  m_inOAngle=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("outputAngle"));
  m_inOCtr  =inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("outputCtr"));
  m_inRlp=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("rlp"));
  m_inSap=inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("sap"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_kaleidoscope :: ~pix_kaleidoscope()
{ 
  Pete_Kaleidoscope_DeInit();

  inlet_free(m_inDiv);
  inlet_free(m_inSAngle);
  inlet_free(m_inSCtr);
  inlet_free(m_inOAngle);
  inlet_free(m_inOCtr);
  inlet_free(m_inRlp);
  inlet_free(m_inSap);
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_kaleidoscope :: processRGBAImage(imageStruct &image)
{
  nWidth = image.xsize;
  nHeight = image.ysize;
  //const int nWidth=pInstanceData->nWidth;
  //const int nHeight=pInstanceData->nHeight;
  if (!init) {
    Pete_Kaleidoscope_Init();
    init = 1;
  }
  pSource = reinterpret_cast<U32*>(image.data);

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.setCsizeByFormat(image.format);
  myImage.reallocate();
  pOutput = reinterpret_cast<U32*>(myImage.data);

  if (m_Divisions<1.0f) {

    const int nByteCount=(nWidth*nHeight*sizeof(U32));
    memcpy(pOutput,pSource,nByteCount);

    return;
  } else if (m_Divisions<2.0f) {
    //SPete_SimpleMirror_Data SimpleMirrorData;
    //SimpleMirrorData.nWidth=nWidth;
    //SimpleMirrorData.nHeight=nHeight;

    m_Angle=(m_OutputAnglePreIncrement/Pete_TwoPi)*360.0f;
    m_DoSimpleMirrorAll=0.0f;
    m_PlaneD=0.0f;

    Pete_SimpleMirror_Render();

    return;
  }

  int nLinesCount;
  Pete_Kaleidoscope_SetupLines(&nLinesCount);

  SPete_Kaleidoscope_Line* pLinesStart=reinterpret_cast<SPete_Kaleidoscope_Line*>(Pete_LockHandle(hLines));
  if (pLinesStart==NULL)return;

  const float Width=static_cast<float>(nWidth);
  //const float HalfWidth=(Width/2.0f);
  const float Height=static_cast<float>(nHeight);
  //const float HalfHeight=(Height/2.0f);

  const float SourceStartAngle=m_SourceAnglePreIncrement;
  float SourceHalfAngle=(Pete_TwoPi/(ceilf(m_Divisions)*2.0f));
  SourceHalfAngle*=m_SourceAngleProportion;
  SourceHalfAngle+=SourceStartAngle;

  const float StartUOffset=(m_SourceCentreX*Width);
  const float StartUGradient=cos(SourceStartAngle);
  const float StartVOffset=(m_SourceCentreY*Height);
  const float StartVGradient=sin(SourceStartAngle);

  const float HalfUOffset=(m_SourceCentreX*Width);
  const float HalfUGradient=cos(SourceHalfAngle);
  const float HalfVOffset=(m_SourceCentreY*Height);
  const float HalfVGradient=sin(SourceHalfAngle);

  SPete_Kaleidoscope_PartitionData PartitionData;
  Pete_Kaleidoscope_PartitionLines(pLinesStart,nLinesCount,&PartitionData);
	
  const float OutputCentreX=(m_OutputCentreX*(Width-1));
  const float OutputCentreY=(m_OutputCentreY*(Height-1));

  const float LeftX=-OutputCentreX;
  const float RightX=Width-OutputCentreX;

  float CurrentY=-OutputCentreY;

  int nScanLine;
  for (nScanLine=0; nScanLine<nHeight; nScanLine+=1) {
    SPete_Kaleidoscope_Line* pLinesGroupStart;
    int nLinesGroupCount;
    SPete_Kaleidoscope_Line* pFirstLineOtherGroup;
    SPete_Kaleidoscope_Line* pLastLineOtherGroup;

    if (CurrentY<0.0f) {
      pLinesGroupStart=PartitionData.pYNegLines;
      nLinesGroupCount=PartitionData.nYNegLinesCount;
      pFirstLineOtherGroup=PartitionData.pYPosLines;
      pLastLineOtherGroup=
	PartitionData.pYPosLines+(PartitionData.nYPosLinesCount-1);
    } else {
      pLinesGroupStart=PartitionData.pYPosLines;
      nLinesGroupCount=PartitionData.nYPosLinesCount;
      pFirstLineOtherGroup=PartitionData.pYNegLines;
      pLastLineOtherGroup=
	PartitionData.pYNegLines+(PartitionData.nYNegLinesCount-1);
    }

    SPete_Kaleidoscope_Line* pLinesGroupEnd=pLinesGroupStart+nLinesGroupCount;
    SPete_Kaleidoscope_Line* pCurrentLine=pLinesGroupStart;

    float PreviousIntersectionX = 0.0f;
    float PreviousRowU = 0.0f;
    float PreviousRowV = 0.0f;

    U32* pOutputLineStart=pOutput+(nScanLine*nWidth);

    while ((pCurrentLine<=pLinesGroupEnd)&&(PreviousIntersectionX<RightX)) {
      const bool bIsFinalSpan=(pCurrentLine==pLinesGroupEnd);
      const bool bIsFirstSpan=(pCurrentLine==pLinesGroupStart);
      float IntersectionX;
      float IntersectionT;

      if (bIsFinalSpan) {
	IntersectionT=0.0f;
	IntersectionX=RightX;
      } else if (fabsf(pCurrentLine->Y)<Pete_Kaleidoscope_Epsilon) {
	if (pCurrentLine->Y<0.0f)
	  IntersectionT=CurrentY/-Pete_Kaleidoscope_Epsilon;
	else 
	  IntersectionT=CurrentY/Pete_Kaleidoscope_Epsilon;
	IntersectionX=(pCurrentLine->X*IntersectionT);
      } else {
	IntersectionT=(CurrentY/pCurrentLine->Y);
	IntersectionX=(pCurrentLine->X*IntersectionT);
      }

      float RowEndU;
      float RowEndV;
      bool bDebugIsHalfLine;
			
      if (bIsFirstSpan) {
	SPete_Kaleidoscope_Line* pLine1=pFirstLineOtherGroup;
	SPete_Kaleidoscope_Line* pLine2=pCurrentLine;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine1->X<0.0f)
	    Line1IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  else
	    Line1IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  Line1IntersectionY=(pLine1->Y*IntersectionT);
	} else {
	  Line1IntersectionT=(LeftX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine2->X<0.0f)
	    Line2IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  else
	    Line2IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(LeftX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;
	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=(Line2IntersectionY-Line1IntersectionY);
	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	PreviousRowU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	PreviousRowV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon)
	  if (pLine1->X<0.0f)
	    PreviousIntersectionX=-Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	  else
	    PreviousIntersectionX=Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	else
	  PreviousIntersectionX=(pLine1->X*Line1IntersectionT);

	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}

      } else if (bIsFinalSpan) {
	SPete_Kaleidoscope_Line* pLine1=(pCurrentLine-1);
	SPete_Kaleidoscope_Line* pLine2=pLastLineOtherGroup;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  Line1IntersectionT=10000.0f;
	  Line1IntersectionY=(pLine1->Y*10000.0f);
	} else {
	  Line1IntersectionT=(RightX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  Line2IntersectionT=10000.0f;
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(RightX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=(Line2IntersectionY-Line1IntersectionY);
	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	RowEndU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	RowEndV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

      } else {
		
	bool bIsHalfLine=(pCurrentLine->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}
      }

      if (IntersectionX>LeftX) {
	int nRowStartX;
	if (PreviousIntersectionX<LeftX) {
	  nRowStartX=0;

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(LeftX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  PreviousRowU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  PreviousRowV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else nRowStartX=static_cast<int>(PreviousIntersectionX-LeftX);
	
	int nRowEndX;

	if (bIsFinalSpan) {
	  nRowEndX=static_cast<int>(RightX-LeftX);
	} else if (IntersectionX>RightX) {
	  nRowEndX=static_cast<int>(RightX-LeftX);

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(RightX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  RowEndU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  RowEndV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else
	  nRowEndX=static_cast<int>(IntersectionX-LeftX);
				
	U32* pRowStart=pOutputLineStart+nRowStartX;
	int nRowLength=(nRowEndX-nRowStartX);
	if (nRowLength<=0) nRowLength=1;
	U32*const pSpanEnd=(pRowStart+nRowLength);
	
	const int nFPShift=16;
	const int nFPMult=(1<<nFPShift);

	float CurrentU=PreviousRowU;
	float CurrentV=PreviousRowV;
	float DeltaU  =(RowEndU-PreviousRowU)/nRowLength;
	float DeltaV  =(RowEndV-PreviousRowV)/nRowLength;

	int nCurrentU=static_cast<int>(CurrentU*nFPMult);
	int nCurrentV=static_cast<int>(CurrentV*nFPMult);

	const int nWidthFP=(nWidth<<nFPShift);
	const int nHeightFP=(nHeight<<nFPShift);

	nCurrentU+=(nWidthFP*10);
	nCurrentV+=(nHeightFP*10);

	int nDeltaU=static_cast<int>(DeltaU*nFPMult);
	int nDeltaV=static_cast<int>(DeltaV*nFPMult);

	const int nTwoWidth=(nWidth*2)<<nFPShift;
	const int nTwoWidthMinusOne=(nTwoWidth-(1<<nFPShift));

	const int nTwoHeight=(nHeight*2)<<nFPShift;
	const int nTwoHeightMinusOne=(nTwoHeight-(1<<nFPShift));

	U32* pCurrentOutput=pRowStart;
	while (pCurrentOutput<pSpanEnd) {
	  int nNextU;
	  if (nDeltaU>=0) nNextU=((nCurrentU+nWidthFP)/nWidthFP)*nWidthFP;
	  else nNextU=((nCurrentU-(1<<nFixedShift))/nWidthFP)*nWidthFP;
	  
	  int nUDist;
	  if (nDeltaU!=0) {
	    nUDist=(nNextU-nCurrentU)/nDeltaU;
	    nUDist+=1;
	  } else nUDist=cnBiggestSignedInt;

	  int nNextV;
	  if (nDeltaV>=0)  nNextV=((nCurrentV+nHeightFP)/nHeightFP)*nHeightFP;
	  else nNextV=((nCurrentV-(1<<nFixedShift))/nHeightFP)*nHeightFP;
	  
	  int nVDist;
	  if (nDeltaV!=0) {
	    nVDist=(nNextV-nCurrentV)/nDeltaV;
	    nVDist+=1;
	  } else nVDist=cnBiggestSignedInt;
	  
	  int nMinDist = (nUDist<nVDist)?nUDist:nVDist;

	  int nStartU=nCurrentU%nTwoWidth;
	  if (nStartU>=nWidthFP)
	    nStartU=(nTwoWidthMinusOne-nStartU);
	  nStartU=clampFunc(nStartU,0,nWidthFP-(1<<nFPShift));

	  int nStartV=nCurrentV%nTwoHeight;
	  if (nStartV>=nHeightFP)
	    nStartV=(nTwoHeightMinusOne-nStartV);
	  nStartV=clampFunc(nStartV,0,nHeightFP-(1<<nFPShift));
	  
	  int nEndU=(nCurrentU+(nMinDist*nDeltaU))%nTwoWidth;
	  if (nEndU>=nWidthFP)
	    nEndU=(nTwoWidthMinusOne-nEndU);
	  nEndU=clampFunc(nEndU,0,nWidthFP-(1<<nFPShift));

	  int nEndV=(nCurrentV+(nMinDist*nDeltaV))%nTwoHeight;
	  if (nEndV>=nHeightFP)
	    nEndV=(nTwoHeightMinusOne-nEndV);
	  nEndV=clampFunc(nEndV,0,nHeightFP-(1<<nFPShift));

	  int nLocalDeltaU;
	  int nLocalDeltaV;
	  if (nMinDist<1) {
	    nLocalDeltaU=0;
	    nLocalDeltaV=0;
	  } else {
	    nLocalDeltaU=(nEndU/nMinDist)-(nStartU/nMinDist);
	    nLocalDeltaV=(nEndV/nMinDist)-(nStartV/nMinDist);
	  }
	  int nLocalCurrentU=nStartU;
	  int nLocalCurrentV=nStartV;

	  U32* pLocalSpanEnd=(pCurrentOutput+nMinDist);
	  if ((pLocalSpanEnd>pSpanEnd)||(nMinDist==cnBiggestSignedInt)) {
	    pLocalSpanEnd=pSpanEnd;
	  }

	  while (pCurrentOutput<pLocalSpanEnd) {
	    const int nUIntegral=(nLocalCurrentU>>nFPShift);
	    const int nVIntegral=(nLocalCurrentV>>nFPShift);
						
	    U32* pCurrentSource=
	      pSource+(nVIntegral*nWidth)+nUIntegral;

	    *pCurrentOutput=*pCurrentSource;

	    pCurrentOutput+=1;
	    nLocalCurrentU+=nLocalDeltaU;
	    nLocalCurrentV+=nLocalDeltaV;
	  }

	  if (nMinDist<1) {
	    nCurrentU+=nDeltaU;
	    nCurrentV+=nDeltaV;
	  } else {
	    nCurrentU+=(nMinDist*nDeltaU);
	    nCurrentV+=(nMinDist*nDeltaV);
	  }
	}
      }

      PreviousIntersectionX=IntersectionX;
      PreviousRowU=RowEndU;
      PreviousRowV=RowEndV;

      pCurrentLine+=1;
    }

    CurrentY+=1.0f;
  }
  image.data = myImage.data;
}
/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_kaleidoscope :: processYUVImage(imageStruct &image)
{
  nWidth = image.xsize/2;
  nHeight = image.ysize;
    
  if (!init) {
    Pete_Kaleidoscope_Init();
    init = 1;
  }
  pSource = reinterpret_cast<U32*>(image.data);
  /*  works
  if ( myImage.xsize*myImage.ysize*myImage.csize != image.xsize*image.ysize*image.csize ){
    int dataSize = image.xsize * image.ysize * image.csize;
    myImage.clear();

    myImage.allocate(dataSize);
  }
*/

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;

  myImage.csize = image.csize;
  myImage.type  = image.type; 
  //this is perhaps buggy
  //myImage.setCsizeByFormat(image.format);
  myImage.reallocate();

  pOutput = reinterpret_cast<U32*>(myImage.data);

  if (m_Divisions<1.0f) {
    const int nByteCount=(nWidth*nHeight*sizeof(U32));
    memcpy(pOutput,pSource,nByteCount);
    return;
  } else if (m_Divisions<2.0f) {
    /*SPete_SimpleMirror_Data SimpleMirrorData;
      SimpleMirrorData.nWidth=nWidth;
      SimpleMirrorData.nHeight=nHeight;*/

    //SPete_SimpleMirror_Settings SimpleMirrorSettings;
    m_Angle=(m_OutputAnglePreIncrement/Pete_TwoPi)*360.0f;
    m_DoSimpleMirrorAll=0.0f;
    m_PlaneD=0.0f;

    Pete_SimpleMirror_Render();
    return;
  }

  int nLinesCount;
  Pete_Kaleidoscope_SetupLines(&nLinesCount);

  SPete_Kaleidoscope_Line* pLinesStart=reinterpret_cast<SPete_Kaleidoscope_Line*>(Pete_LockHandle(hLines));
  if (pLinesStart==NULL)return;

  const float Width=static_cast<float>(nWidth);
  //const float HalfWidth=(Width/2.0f);
  const float Height=static_cast<float>(nHeight);
  //const float HalfHeight=(Height/2.0f);

  const float SourceStartAngle = m_SourceAnglePreIncrement;
  float SourceHalfAngle = (Pete_TwoPi/(ceilf(m_Divisions)*2.0f));
  SourceHalfAngle *= m_SourceAngleProportion;
  SourceHalfAngle += SourceStartAngle;

  const float StartUOffset=(m_SourceCentreX*Width);
  const float StartUGradient=cos(SourceStartAngle);
  const float StartVOffset=(m_SourceCentreY*Height);
  const float StartVGradient=sin(SourceStartAngle);

  const float HalfUOffset=(m_SourceCentreX*Width);
  const float HalfUGradient=cos(SourceHalfAngle);
  const float HalfVOffset=(m_SourceCentreY*Height);
  const float HalfVGradient=sin(SourceHalfAngle);

  SPete_Kaleidoscope_PartitionData PartitionData;
  Pete_Kaleidoscope_PartitionLines(pLinesStart,nLinesCount,&PartitionData);
	
  const float OutputCentreX=(m_OutputCentreX*(Width-1));
  const float OutputCentreY=(m_OutputCentreY*(Height-1));

  const float LeftX=-OutputCentreX;
  const float RightX=Width-OutputCentreX;

  float CurrentY=-OutputCentreY;
  int nScanLine;
  for (nScanLine=0; nScanLine<nHeight; nScanLine+=1) {
    SPete_Kaleidoscope_Line* pLinesGroupStart;
    int nLinesGroupCount;
    SPete_Kaleidoscope_Line* pFirstLineOtherGroup;
    SPete_Kaleidoscope_Line* pLastLineOtherGroup;

    if (CurrentY<0.0f) {
      pLinesGroupStart=PartitionData.pYNegLines;
      nLinesGroupCount=PartitionData.nYNegLinesCount;
      pFirstLineOtherGroup=PartitionData.pYPosLines;
      pLastLineOtherGroup=
	PartitionData.pYPosLines+(PartitionData.nYPosLinesCount-1);
    } else {
      pLinesGroupStart=PartitionData.pYPosLines;
      nLinesGroupCount=PartitionData.nYPosLinesCount;
      pFirstLineOtherGroup=PartitionData.pYNegLines;
      pLastLineOtherGroup=
	PartitionData.pYNegLines+(PartitionData.nYNegLinesCount-1);
    }

    SPete_Kaleidoscope_Line* pLinesGroupEnd=pLinesGroupStart+nLinesGroupCount;

    SPete_Kaleidoscope_Line* pCurrentLine=pLinesGroupStart;

    float PreviousIntersectionX = 0.0f;
    float PreviousRowU = 0.0f;
    float PreviousRowV = 0.0f;

    U32* pOutputLineStart=pOutput+(nScanLine*nWidth);
    while ((pCurrentLine<=pLinesGroupEnd)&&(PreviousIntersectionX<RightX)) {
      const bool bIsFinalSpan=(pCurrentLine==pLinesGroupEnd);
      const bool bIsFirstSpan=(pCurrentLine==pLinesGroupStart);
			
      float IntersectionX;
      float IntersectionT;
      if (bIsFinalSpan) {
	IntersectionT=0.0f;
	IntersectionX=RightX;
      } else if (fabsf(pCurrentLine->Y)<Pete_Kaleidoscope_Epsilon) {
	if (pCurrentLine->Y<0.0f) {
	  IntersectionT=CurrentY/-Pete_Kaleidoscope_Epsilon;
	} else {
	  IntersectionT=CurrentY/Pete_Kaleidoscope_Epsilon;
	}
	IntersectionX=(pCurrentLine->X*IntersectionT);
      } else {
	IntersectionT=(CurrentY/pCurrentLine->Y);
	IntersectionX=(pCurrentLine->X*IntersectionT);
      }

      float RowEndU;
      float RowEndV;

      bool bDebugIsHalfLine;
      if (bIsFirstSpan) {
	SPete_Kaleidoscope_Line* pLine1=pFirstLineOtherGroup;
	SPete_Kaleidoscope_Line* pLine2=pCurrentLine;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine1->X<0.0f) {
	    Line1IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  } else {
	    Line1IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  }
	  Line1IntersectionY=(pLine1->Y*IntersectionT);
	} else {
	  Line1IntersectionT=(LeftX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine2->X<0.0f) {
	    Line2IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  } else {
	    Line2IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  }
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(LeftX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;
	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=
	  (Line2IntersectionY-Line1IntersectionY);

	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	PreviousRowU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	PreviousRowV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine1->X<0.0f) {
	    PreviousIntersectionX=-Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	  } else {
	    PreviousIntersectionX=Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	  }
	} else {
	  PreviousIntersectionX=(pLine1->X*Line1IntersectionT);
	}
				
	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}

      } else if (bIsFinalSpan) {
	SPete_Kaleidoscope_Line* pLine1=(pCurrentLine-1);
	SPete_Kaleidoscope_Line* pLine2=pLastLineOtherGroup;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  Line1IntersectionT=10000.0f;
	  Line1IntersectionY=(pLine1->Y*10000.0f);
	} else {
	  Line1IntersectionT=(RightX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  Line2IntersectionT=10000.0f;
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(RightX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

				
	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=
	  (Line2IntersectionY-Line1IntersectionY);

	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	RowEndU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	RowEndV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

      } else {
	bool bIsHalfLine=(pCurrentLine->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}

      }
      if (IntersectionX>LeftX) {
	int nRowStartX;
	
	if (PreviousIntersectionX<LeftX) {
	  nRowStartX=0;

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(LeftX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  PreviousRowU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  PreviousRowV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else {
	  nRowStartX=static_cast<int>(PreviousIntersectionX-LeftX);
	}

	int nRowEndX;
	if (bIsFinalSpan) {
	  nRowEndX=static_cast<int>(RightX-LeftX);
	} else if (IntersectionX>RightX) {
	  nRowEndX=static_cast<int>(RightX-LeftX);

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(RightX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  RowEndU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  RowEndV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else {
	  nRowEndX=static_cast<int>(IntersectionX-LeftX);
	}
		
	U32* pRowStart=pOutputLineStart+nRowStartX;
	int nRowLength=(nRowEndX-nRowStartX);
	if (nRowLength<=0) {
	  nRowLength=1;
	}
	U32*const pSpanEnd=(pRowStart+nRowLength);

	const int nFPShift=16;
	const int nFPMult=(1<<nFPShift);

	float CurrentU=PreviousRowU;
	float CurrentV=PreviousRowV;

	float DeltaU=(RowEndU-PreviousRowU)/nRowLength;
	float DeltaV=(RowEndV-PreviousRowV)/nRowLength;

	int nCurrentU=static_cast<int>(CurrentU*nFPMult);
	int nCurrentV=static_cast<int>(CurrentV*nFPMult);

	const int nWidthFP=(nWidth<<nFPShift);
	const int nHeightFP=(nHeight<<nFPShift);

	nCurrentU+=(nWidthFP*10);
	nCurrentV+=(nHeightFP*10);

	int nDeltaU=static_cast<int>(DeltaU*nFPMult);
	int nDeltaV=static_cast<int>(DeltaV*nFPMult);

	const int nTwoWidth=(nWidth*2)<<nFPShift;
	const int nTwoWidthMinusOne=(nTwoWidth-(1<<nFPShift));

	const int nTwoHeight=(nHeight*2)<<nFPShift;
	const int nTwoHeightMinusOne=(nTwoHeight-(1<<nFPShift));

	U32* pCurrentOutput=pRowStart;

	while (pCurrentOutput<pSpanEnd) {
	  int nNextU;
	  if (nDeltaU>=0) {
	    nNextU=((nCurrentU+nWidthFP)/nWidthFP)*nWidthFP;
	  } else {
	    nNextU=((nCurrentU-(1<<nFixedShift))/nWidthFP)*nWidthFP;
	  }

	  int nUDist;
	  if (nDeltaU!=0) {
	    nUDist=(nNextU-nCurrentU)/nDeltaU;
	    nUDist+=1;
	  } else {
	    nUDist=cnBiggestSignedInt;
	  }
	  int nNextV;
	  if (nDeltaV>=0) {
	    nNextV=((nCurrentV+nHeightFP)/nHeightFP)*nHeightFP;
	  } else {
	    nNextV=((nCurrentV-(1<<nFixedShift))/nHeightFP)*nHeightFP;
	  }
	  int nVDist;
	  if (nDeltaV!=0) {
	    nVDist=(nNextV-nCurrentV)/nDeltaV;
	    nVDist+=1;
	  } else {
	    nVDist=cnBiggestSignedInt;
	  }

	  int nMinDist;
	  if (nUDist<nVDist) {
	    nMinDist=nUDist;
	  } else {
	    nMinDist=nVDist;
	  }

	  int nStartU=nCurrentU%nTwoWidth;
	  if (nStartU>=nWidthFP) {
	    nStartU=(nTwoWidthMinusOne-nStartU);
	  }
	  nStartU=clampFunc(nStartU,0,nWidthFP-(1<<nFPShift));

	  int nStartV=nCurrentV%nTwoHeight;
	  if (nStartV>=nHeightFP) {
	    nStartV=(nTwoHeightMinusOne-nStartV);
	  }
	  nStartV=clampFunc(nStartV,0,nHeightFP-(1<<nFPShift));

	  int nEndU=(nCurrentU+(nMinDist*nDeltaU))%nTwoWidth;
	  if (nEndU>=nWidthFP) {
	    nEndU=(nTwoWidthMinusOne-nEndU);
	  }
	  nEndU=clampFunc(nEndU,0,nWidthFP-(1<<nFPShift));

	  int nEndV=(nCurrentV+(nMinDist*nDeltaV))%nTwoHeight;
	  if (nEndV>=nHeightFP) {
	    nEndV=(nTwoHeightMinusOne-nEndV);
	  }
	  nEndV=clampFunc(nEndV,0,nHeightFP-(1<<nFPShift));

	  int nLocalDeltaU;
	  int nLocalDeltaV;
	  if (nMinDist<1) {
	    nLocalDeltaU=0;
	    nLocalDeltaV=0;
	  } else {
	    nLocalDeltaU=(nEndU/nMinDist)-(nStartU/nMinDist);
	    nLocalDeltaV=(nEndV/nMinDist)-(nStartV/nMinDist);
	  }
	  int nLocalCurrentU=nStartU;
	  int nLocalCurrentV=nStartV;

	  U32* pLocalSpanEnd=(pCurrentOutput+nMinDist);
	  if ((pLocalSpanEnd>pSpanEnd)||(nMinDist==cnBiggestSignedInt)) {
	    pLocalSpanEnd=pSpanEnd;
	  }
	  while (pCurrentOutput<pLocalSpanEnd) {
	    const int nUIntegral=(nLocalCurrentU>>nFPShift);
	    const int nVIntegral=(nLocalCurrentV>>nFPShift);
	    U32* pCurrentSource=
	      pSource+(nVIntegral*nWidth)+nUIntegral;
	    *pCurrentOutput=*pCurrentSource;
	    pCurrentOutput+=1;
	    nLocalCurrentU+=nLocalDeltaU;
	    nLocalCurrentV+=nLocalDeltaV;
	  }
	  if (nMinDist<1) {
	    nCurrentU+=nDeltaU;
	    nCurrentV+=nDeltaV;
	  } else {
	    nCurrentU+=(nMinDist*nDeltaU);
	    nCurrentV+=(nMinDist*nDeltaV);
	  }
	}
      }
      PreviousIntersectionX=IntersectionX;
      PreviousRowU=RowEndU;
      PreviousRowV=RowEndV;

      pCurrentLine+=1;
			
    }

    CurrentY+=1.0f;

  }
  image.data = myImage.data;
}
/////////////////////////////////////////////////////////
// do the Grey processing here
//
/////////////////////////////////////////////////////////
void pix_kaleidoscope :: processGrayImage(imageStruct &image)
{
  nWidth = image.xsize;
  nHeight = image.ysize;
    
  if (!init) {
    Pete_Kaleidoscope_Init();
    init = 1;
  }
  unsigned char* pSource = image.data;

  myImage.xsize = image.xsize;
  myImage.ysize = image.ysize;
  myImage.setCsizeByFormat(image.format);
  myImage.reallocate();
  unsigned char* pOutput = myImage.data;

  if (m_Divisions<1.0f) {
    const int nByteCount=(nWidth*nHeight*sizeof(unsigned char));
    memcpy(pOutput,pSource,nByteCount);
    return;
  } else if (m_Divisions<2.0f) {
    /*SPete_SimpleMirror_Data SimpleMirrorData;
      SimpleMirrorData.nWidth=nWidth;
      SimpleMirrorData.nHeight=nHeight;*/

    //SPete_SimpleMirror_Settings SimpleMirrorSettings;
    m_Angle=(m_OutputAnglePreIncrement/Pete_TwoPi)*360.0f;
    m_DoSimpleMirrorAll=0.0f;
    m_PlaneD=0.0f;

    Pete_SimpleMirror_Render();

    return;

  }

  int nLinesCount;
  Pete_Kaleidoscope_SetupLines(&nLinesCount);

  SPete_Kaleidoscope_Line* pLinesStart=reinterpret_cast<SPete_Kaleidoscope_Line*>(Pete_LockHandle(hLines));
  if (pLinesStart==NULL) {
    return;
  }

  const float Width=static_cast<float>(nWidth);
  //const float HalfWidth=(Width/2.0f);
  const float Height=static_cast<float>(nHeight);
  //const float HalfHeight=(Height/2.0f);

  const float SourceStartAngle = m_SourceAnglePreIncrement;
  float SourceHalfAngle = (Pete_TwoPi/(ceilf(m_Divisions)*2.0f));
  SourceHalfAngle *= m_SourceAngleProportion;
  SourceHalfAngle += SourceStartAngle;

  const float StartUOffset=(m_SourceCentreX*Width);
  const float StartUGradient=cos(SourceStartAngle);
  const float StartVOffset=(m_SourceCentreY*Height);
  const float StartVGradient=sin(SourceStartAngle);

  const float HalfUOffset=(m_SourceCentreX*Width);
  const float HalfUGradient=cos(SourceHalfAngle);
  const float HalfVOffset=(m_SourceCentreY*Height);
  const float HalfVGradient=sin(SourceHalfAngle);

  SPete_Kaleidoscope_PartitionData PartitionData;
  Pete_Kaleidoscope_PartitionLines(pLinesStart,nLinesCount,&PartitionData);
	
  const float OutputCentreX=(m_OutputCentreX*(Width-1));
  const float OutputCentreY=(m_OutputCentreY*(Height-1));

  const float LeftX=-OutputCentreX;
  const float RightX=Width-OutputCentreX;

  float CurrentY=-OutputCentreY;
  int nScanLine;
  for (nScanLine=0; nScanLine<nHeight; nScanLine+=1) {
    SPete_Kaleidoscope_Line* pLinesGroupStart;
    int nLinesGroupCount;
    SPete_Kaleidoscope_Line* pFirstLineOtherGroup;
    SPete_Kaleidoscope_Line* pLastLineOtherGroup;

    if (CurrentY<0.0f) {
      pLinesGroupStart=PartitionData.pYNegLines;
      nLinesGroupCount=PartitionData.nYNegLinesCount;
      pFirstLineOtherGroup=PartitionData.pYPosLines;
      pLastLineOtherGroup=
	PartitionData.pYPosLines+(PartitionData.nYPosLinesCount-1);
    } else {
      pLinesGroupStart=PartitionData.pYPosLines;
      nLinesGroupCount=PartitionData.nYPosLinesCount;
      pFirstLineOtherGroup=PartitionData.pYNegLines;
      pLastLineOtherGroup=
	PartitionData.pYNegLines+(PartitionData.nYNegLinesCount-1);
    }

    SPete_Kaleidoscope_Line* pLinesGroupEnd=pLinesGroupStart+nLinesGroupCount;

    SPete_Kaleidoscope_Line* pCurrentLine=pLinesGroupStart;

    float PreviousIntersectionX = 0.0f;
    float PreviousRowU = 0.0f;
    float PreviousRowV = 0.0f;

    unsigned char* pOutputLineStart=pOutput+(nScanLine*nWidth);
    while ((pCurrentLine<=pLinesGroupEnd)&&(PreviousIntersectionX<RightX)) {
      const bool bIsFinalSpan=(pCurrentLine==pLinesGroupEnd);
      const bool bIsFirstSpan=(pCurrentLine==pLinesGroupStart);
			
      float IntersectionX;
      float IntersectionT;
      if (bIsFinalSpan) {
	IntersectionT=0.0f;
	IntersectionX=RightX;
      } else if (fabsf(pCurrentLine->Y)<Pete_Kaleidoscope_Epsilon) {
	if (pCurrentLine->Y<0.0f) {
	  IntersectionT=CurrentY/-Pete_Kaleidoscope_Epsilon;
	} else {
	  IntersectionT=CurrentY/Pete_Kaleidoscope_Epsilon;
	}
	IntersectionX=(pCurrentLine->X*IntersectionT);
      } else {
	IntersectionT=(CurrentY/pCurrentLine->Y);
	IntersectionX=(pCurrentLine->X*IntersectionT);
      }

      float RowEndU;
      float RowEndV;

      bool bDebugIsHalfLine;
      if (bIsFirstSpan) {
	SPete_Kaleidoscope_Line* pLine1=pFirstLineOtherGroup;
	SPete_Kaleidoscope_Line* pLine2=pCurrentLine;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine1->X<0.0f) {
	    Line1IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  } else {
	    Line1IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  }
	  Line1IntersectionY=(pLine1->Y*IntersectionT);
	} else {
	  Line1IntersectionT=(LeftX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine2->X<0.0f) {
	    Line2IntersectionT=LeftX/-Pete_Kaleidoscope_Epsilon;
	  } else {
	    Line2IntersectionT=LeftX/Pete_Kaleidoscope_Epsilon;
	  }
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(LeftX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;
	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=
	  (Line2IntersectionY-Line1IntersectionY);

	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	PreviousRowU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	PreviousRowV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  if (pLine1->X<0.0f) {
	    PreviousIntersectionX=-Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	  } else {
	    PreviousIntersectionX=Pete_Kaleidoscope_Epsilon*Line1IntersectionT;
	  }
	} else {
	  PreviousIntersectionX=(pLine1->X*Line1IntersectionT);
	}
				
	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}

      } else if (bIsFinalSpan) {
	SPete_Kaleidoscope_Line* pLine1=(pCurrentLine-1);
	SPete_Kaleidoscope_Line* pLine2=pLastLineOtherGroup;

	float Line1IntersectionY;
	float Line1IntersectionT;
	if (fabsf(pLine1->X)<Pete_Kaleidoscope_Epsilon) {
	  Line1IntersectionT=10000.0f;
	  Line1IntersectionY=(pLine1->Y*10000.0f);
	} else {
	  Line1IntersectionT=(RightX/pLine1->X);
	  Line1IntersectionY=(pLine1->Y*Line1IntersectionT);
	}

	float Line2IntersectionY;
	float Line2IntersectionT;
	if (fabsf(pLine2->X)<Pete_Kaleidoscope_Epsilon) {
	  Line2IntersectionT=10000.0f;
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	} else {
	  Line2IntersectionT=(RightX/pLine2->X);
	  Line2IntersectionY=(pLine2->Y*Line2IntersectionT);
	}

				
	bool bIsHalfLine=(pLine2->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	float Line1U;
	float Line1V;
	float Line2U;
	float Line2V;
	if (bIsHalfLine) {
	  Line1U=StartUOffset+(Line1IntersectionT*StartUGradient);
	  Line1V=StartVOffset+(Line1IntersectionT*StartVGradient);
	  Line2U=HalfUOffset+(Line2IntersectionT*HalfUGradient);
	  Line2V=HalfVOffset+(Line2IntersectionT*HalfVGradient);
	} else {
	  Line1U=HalfUOffset+(Line1IntersectionT*HalfUGradient);
	  Line1V=HalfVOffset+(Line1IntersectionT*HalfVGradient);
	  Line2U=StartUOffset+(Line2IntersectionT*StartUGradient);
	  Line2V=StartVOffset+(Line2IntersectionT*StartVGradient);
	}

	const float YDist=
	  (Line2IntersectionY-Line1IntersectionY);

	const float OneMinusLerpValue=(CurrentY-Line1IntersectionY)/YDist;
	const float LerpValue=(1.0f-OneMinusLerpValue);

	RowEndU=(Line1U*LerpValue)+(Line2U*OneMinusLerpValue);
	RowEndV=(Line1V*LerpValue)+(Line2V*OneMinusLerpValue);

      } else {
	bool bIsHalfLine=(pCurrentLine->Flags&PETE_KALEIDOSCOPE_HALFLINE_BIT);
	bDebugIsHalfLine=bIsHalfLine;

	if (bIsHalfLine) {
	  RowEndU=HalfUOffset+(IntersectionT*HalfUGradient);
	  RowEndV=HalfVOffset+(IntersectionT*HalfVGradient);
	} else {
	  RowEndU=StartUOffset+(IntersectionT*StartUGradient);
	  RowEndV=StartVOffset+(IntersectionT*StartVGradient);
	}

      }
      if (IntersectionX>LeftX) {
	int nRowStartX;
	
	if (PreviousIntersectionX<LeftX) {
	  nRowStartX=0;

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(LeftX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  PreviousRowU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  PreviousRowV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else {
	  nRowStartX=static_cast<int>(PreviousIntersectionX-LeftX);
	}

	int nRowEndX;
	if (bIsFinalSpan) {
	  nRowEndX=static_cast<int>(RightX-LeftX);
	} else if (IntersectionX>RightX) {
	  nRowEndX=static_cast<int>(RightX-LeftX);

	  const float XDist=(IntersectionX-PreviousIntersectionX);
	  const float OneMinusLerpValue=(RightX-PreviousIntersectionX)/XDist;
	  const float LerpValue=(1.0f-OneMinusLerpValue);

	  RowEndU=(PreviousRowU*LerpValue)+(RowEndU*OneMinusLerpValue);
	  RowEndV=(PreviousRowV*LerpValue)+(RowEndV*OneMinusLerpValue);

	} else {
	  nRowEndX=static_cast<int>(IntersectionX-LeftX);
	}
		
	unsigned char* pRowStart=pOutputLineStart+nRowStartX;
	int nRowLength=(nRowEndX-nRowStartX);
	if (nRowLength<=0) {
	  nRowLength=1;
	}
	unsigned char*const pSpanEnd=(pRowStart+nRowLength);

	const int nFPShift=16;
	const int nFPMult=(1<<nFPShift);

	float CurrentU=PreviousRowU;
	float CurrentV=PreviousRowV;

	float DeltaU=(RowEndU-PreviousRowU)/nRowLength;
	float DeltaV=(RowEndV-PreviousRowV)/nRowLength;

	int nCurrentU=static_cast<int>(CurrentU*nFPMult);
	int nCurrentV=static_cast<int>(CurrentV*nFPMult);

	const int nWidthFP=(nWidth<<nFPShift);
	const int nHeightFP=(nHeight<<nFPShift);

	nCurrentU+=(nWidthFP*10);
	nCurrentV+=(nHeightFP*10);

	int nDeltaU=static_cast<int>(DeltaU*nFPMult);
	int nDeltaV=static_cast<int>(DeltaV*nFPMult);

	const int nTwoWidth=(nWidth*2)<<nFPShift;
	const int nTwoWidthMinusOne=(nTwoWidth-(1<<nFPShift));

	const int nTwoHeight=(nHeight*2)<<nFPShift;
	const int nTwoHeightMinusOne=(nTwoHeight-(1<<nFPShift));

	unsigned char* pCurrentOutput=pRowStart;

	while (pCurrentOutput<pSpanEnd) {
	  int nNextU;
	  if (nDeltaU>=0) {
	    nNextU=((nCurrentU+nWidthFP)/nWidthFP)*nWidthFP;
	  } else {
	    nNextU=((nCurrentU-(1<<nFixedShift))/nWidthFP)*nWidthFP;
	  }

	  int nUDist;
	  if (nDeltaU!=0) {
	    nUDist=(nNextU-nCurrentU)/nDeltaU;
	    nUDist+=1;
	  } else {
	    nUDist=cnBiggestSignedInt;
	  }
	  int nNextV;
	  if (nDeltaV>=0) {
	    nNextV=((nCurrentV+nHeightFP)/nHeightFP)*nHeightFP;
	  } else {
	    nNextV=((nCurrentV-(1<<nFixedShift))/nHeightFP)*nHeightFP;
	  }
	  int nVDist;
	  if (nDeltaV!=0) {
	    nVDist=(nNextV-nCurrentV)/nDeltaV;
	    nVDist+=1;
	  } else {
	    nVDist=cnBiggestSignedInt;
	  }

	  int nMinDist;
	  if (nUDist<nVDist) {
	    nMinDist=nUDist;
	  } else {
	    nMinDist=nVDist;
	  }

	  int nStartU=nCurrentU%nTwoWidth;
	  if (nStartU>=nWidthFP) {
	    nStartU=(nTwoWidthMinusOne-nStartU);
	  }
	  nStartU=clampFunc(nStartU,0,nWidthFP-(1<<nFPShift));

	  int nStartV=nCurrentV%nTwoHeight;
	  if (nStartV>=nHeightFP) {
	    nStartV=(nTwoHeightMinusOne-nStartV);
	  }
	  nStartV=clampFunc(nStartV,0,nHeightFP-(1<<nFPShift));

	  int nEndU=(nCurrentU+(nMinDist*nDeltaU))%nTwoWidth;
	  if (nEndU>=nWidthFP) {
	    nEndU=(nTwoWidthMinusOne-nEndU);
	  }
	  nEndU=clampFunc(nEndU,0,nWidthFP-(1<<nFPShift));

	  int nEndV=(nCurrentV+(nMinDist*nDeltaV))%nTwoHeight;
	  if (nEndV>=nHeightFP) {
	    nEndV=(nTwoHeightMinusOne-nEndV);
	  }
	  nEndV=clampFunc(nEndV,0,nHeightFP-(1<<nFPShift));

	  int nLocalDeltaU;
	  int nLocalDeltaV;
	  if (nMinDist<1) {
	    nLocalDeltaU=0;
	    nLocalDeltaV=0;
	  } else {
	    nLocalDeltaU=(nEndU/nMinDist)-(nStartU/nMinDist);
	    nLocalDeltaV=(nEndV/nMinDist)-(nStartV/nMinDist);
	  }
	  int nLocalCurrentU=nStartU;
	  int nLocalCurrentV=nStartV;

	  unsigned char* pLocalSpanEnd=(pCurrentOutput+nMinDist);
	  if ((pLocalSpanEnd>pSpanEnd)||(nMinDist==cnBiggestSignedInt)) {
	    pLocalSpanEnd=pSpanEnd;
	  }
	  while (pCurrentOutput<pLocalSpanEnd) {
	    const int nUIntegral=(nLocalCurrentU>>nFPShift);
	    const int nVIntegral=(nLocalCurrentV>>nFPShift);
	    unsigned char* pCurrentSource=
	      pSource+(nVIntegral*nWidth)+nUIntegral;
	    *pCurrentOutput=*pCurrentSource;
	    pCurrentOutput+=1;
	    nLocalCurrentU+=nLocalDeltaU;
	    nLocalCurrentV+=nLocalDeltaV;
	  }
	  if (nMinDist<1) {
	    nCurrentU+=nDeltaU;
	    nCurrentV+=nDeltaV;
	  } else {
	    nCurrentU+=(nMinDist*nDeltaU);
	    nCurrentV+=(nMinDist*nDeltaV);
	  }
	}
      }
      PreviousIntersectionX=IntersectionX;
      PreviousRowU=RowEndU;
      PreviousRowV=RowEndV;

      pCurrentLine+=1;
			
    }

    CurrentY+=1.0f;

  }
  image.data = myImage.data;
}

inline int pix_kaleidoscope :: Pete_Kaleidoscope_CosFA(int nAngleFA) {
  const int nGatedAngleFA=nAngleFA&nFixedMask;
  const int nEntryIndex=(nGatedAngleFA>>nFAToCosTableShift);

  return g_pCurrentCosTable[nEntryIndex];
}

inline int pix_kaleidoscope :: Pete_Kaleidoscope_SinFA(int nAngleFA) {
  const int nIncrementedAngleFA=(nAngleFA+nHalfPiFA);
  const int nGatedAngleFA=nIncrementedAngleFA&nFixedMask;
  const int nEntryIndex=(nGatedAngleFA>>nFAToCosTableShift);

  return g_pCurrentCosTable[nEntryIndex];
}

int pix_kaleidoscope :: Pete_Kaleidoscope_Init() {
  Pete_Kaleidoscope_DeInit();

  const int nNumberOfPixels=nWidth*nHeight;
  const int nNumberOfBytes=
    nNumberOfPixels*sizeof(SPete_AngleTable_Entry);
  hAngleTable=Pete_NewHandle(nNumberOfBytes);

  if (hAngleTable==NULL) {
    Pete_Kaleidoscope_DeInit();
    return 0;
  }

  const int nCosTableByteCount=nCosTableSize*sizeof(int);
  hCosTable=Pete_NewHandle(nCosTableByteCount);

  if (hCosTable==NULL) {
    Pete_Kaleidoscope_DeInit();
    return 0;
  }

  const int nLinesByteCount=nMaxLines*sizeof(SPete_Kaleidoscope_Line);
  hLines=Pete_NewHandle(nLinesByteCount);

  if (hLines==NULL) {
    Pete_Kaleidoscope_DeInit();
    return 0;
  }

  Pete_Kaleidoscope_SetupAngleTable();
  Pete_Kaleidoscope_SetupCosTable();

  return 1;

}

void pix_kaleidoscope :: Pete_Kaleidoscope_DeInit() {
  if(init){
    if (hAngleTable!=NULL) {
      Pete_FreeHandle(hAngleTable);
      hAngleTable=NULL;
    }

    if (hCosTable!=NULL) {
      Pete_FreeHandle(hCosTable);
      hCosTable=NULL;
    }

    if (hLines!=NULL) {
      Pete_FreeHandle(hLines);
      hLines=NULL;
    }
  }
}

void pix_kaleidoscope :: Pete_Kaleidoscope_SetupAngleTable() {

  SPete_AngleTable_Entry* pTableStart=reinterpret_cast<SPete_AngleTable_Entry*>(Pete_LockHandle(hAngleTable));
  if (pTableStart==NULL) {
    return;
  }

  const float Width=static_cast<float>(nWidth);
  const float HalfWidth=(Width/2.0f);
  const float Height=static_cast<float>(nHeight);
  const float HalfHeight=(Height/2.0f);
  const int nNumPixels=(nWidth*nHeight);

  SPete_AngleTable_Entry* pCurrent=pTableStart;
  const SPete_AngleTable_Entry* pTableEnd=(pTableStart+nNumPixels);

  float YPos=-HalfHeight;
  while (pCurrent!=pTableEnd) {
		
    const SPete_AngleTable_Entry* pLineEnd=pCurrent+nWidth;

    float XPos=-HalfWidth;
    while (pCurrent!=pLineEnd) {

      float Angle=atan2(YPos,XPos);
      if (Angle<0.0f) {
	    Angle+=Pete_TwoPi;
      }
      float Dist=sqrt((XPos*XPos)+(YPos*YPos));

      pCurrent->nAngleFA=static_cast<int>(Angle*FixedAngleMult);
      pCurrent->nDist=static_cast<int>(Dist*nDistanceMult);

      XPos+=1.0f;
      pCurrent+=1;

    }

    YPos+=1.0f;

  }

}

void pix_kaleidoscope :: Pete_Kaleidoscope_SetupCosTable() {

  int* pTableStart=reinterpret_cast<int*>(Pete_LockHandle(hCosTable));
  if (pTableStart==NULL) {
    return;
  }

  int nCount;
  for (nCount=0; nCount<nCosTableSize; nCount+=1) {

    float Angle=(nCount/static_cast<float>(nCosTableSize))*Pete_TwoPi;

    float Result=cos(Angle);

    pTableStart[nCount]=static_cast<int>(Result*nFixedMult);

  }

}

void pix_kaleidoscope :: Pete_Kaleidoscope_SetupLines(int* poutLinesCount) {

  const float MaxDivisions=((nMaxLines/2)-1.0f);

  float Divisions=ceilf(m_Divisions);
  if (Divisions<1.0f) {
    Divisions=1.0f;
  } else if (Divisions>MaxDivisions) {
    Divisions=MaxDivisions;
  }

  const int nDivisionsInt=static_cast<int>(Divisions);

  const float AnglePreIncrement=m_OutputAnglePreIncrement+0.001;

  const float AngleInterval=(Pete_TwoPi/Divisions);
  const float HalfAngleInterval=(AngleInterval*m_ReflectionLineProportion);

  const int nLinesCount=(nDivisionsInt*2);

  SPete_Kaleidoscope_Line* pLinesStart=reinterpret_cast<SPete_Kaleidoscope_Line*>(Pete_LockHandle(hLines));
  if (pLinesStart==NULL) {
    return;
  }
  //SPete_Kaleidoscope_Line* pLinesEnd=pLinesStart+nLinesCount;

  SPete_Kaleidoscope_Line* pCurrentLine=pLinesStart;

  int nCurrentDivision;
  for (nCurrentDivision=0; nCurrentDivision<nDivisionsInt; nCurrentDivision+=1) {

    float StartAngle=
      AnglePreIncrement+(nCurrentDivision*AngleInterval);
    StartAngle=fmodf(StartAngle,Pete_TwoPi);

    pCurrentLine->X=cos(StartAngle);
    pCurrentLine->Y=sin(StartAngle);
    pCurrentLine->Flags=0;

    pCurrentLine+=1;

    float HalfAngle=(StartAngle+HalfAngleInterval);
    HalfAngle=fmodf(HalfAngle,Pete_TwoPi);

    pCurrentLine->X=cos(HalfAngle);
    pCurrentLine->Y=sin(HalfAngle);
    pCurrentLine->Flags=PETE_KALEIDOSCOPE_HALFLINE_BIT;

    pCurrentLine+=1;

  }

  *poutLinesCount=nLinesCount;
	
}

extern "C" int Pete_Kaleidoscope_LinesSortFunction(const void* pElem1,const void* pElem2) {

  const SPete_Kaleidoscope_Line* pFirstLine;
  pFirstLine = reinterpret_cast<const SPete_Kaleidoscope_Line*>(pElem1);
  const SPete_Kaleidoscope_Line* pSecondLine;
  pSecondLine=reinterpret_cast<const SPete_Kaleidoscope_Line*>(pElem2);

  const float FirstYIsNeg=(pFirstLine->Y<0.0f);
  const float SecondYIsNeg=(pSecondLine->Y<0.0f);

  if (FirstYIsNeg!=SecondYIsNeg) {

    if (FirstYIsNeg) {
      return -1;
    } else {
      return 1;
    }

  } else {

    const float FirstX=pFirstLine->X;
    const float SecondX=pSecondLine->X;

    if (FirstX<SecondX) {
      return -1;
    } else if (FirstX>SecondX) {
      return 1;
    } else {
      return 0;
    }

  }

}

void pix_kaleidoscope :: Pete_Kaleidoscope_PartitionLines(SPete_Kaleidoscope_Line* pLinesStart,int nLinesCount,SPete_Kaleidoscope_PartitionData* poutPartitionData) {

  qsort(reinterpret_cast<void*>(pLinesStart),
	nLinesCount,
	sizeof(SPete_Kaleidoscope_Line),
	&Pete_Kaleidoscope_LinesSortFunction);

  SPete_Kaleidoscope_Line* pLinesEnd=pLinesStart+nLinesCount;

  SPete_Kaleidoscope_Line* pCurrentLine=pLinesStart;
  while ((pCurrentLine<pLinesEnd)&&(pCurrentLine->Y<0.0f)) {
    pCurrentLine+=1;
  }

  const int nYNegCount=pCurrentLine-pLinesStart;
	
  poutPartitionData->pYNegLines=pLinesStart;
  poutPartitionData->nYNegLinesCount=nYNegCount;
  poutPartitionData->pYPosLines=pLinesStart+nYNegCount;
  poutPartitionData->nYPosLinesCount=(nLinesCount-nYNegCount);

}

void pix_kaleidoscope :: Pete_Kaleidoscope_CreateAllTransforms(SPete_2dMatrix* pTransforms) {

  int nDivisionCount=static_cast<int>(m_Divisions);
  nDivisionCount=clampFunc(nDivisionCount,1,(nMaxDivisions-1));

  const float DivisionAngle=(Pete_TwoPi/nDivisionCount);

  const float HalfWidth=nWidth/2.0f;
  const float HalfHeight=nHeight/2.0f;

  SPete_2dMatrix ScreenToWorld;
  Pete_2dMatrix_SetToTranslation(-HalfWidth,-HalfHeight,&ScreenToWorld);

  const float OriginX=m_SourceCentreX;
  const float OriginY=m_SourceCentreY;

  SPete_2dMatrix PanTransform;
  Pete_2dMatrix_SetToTranslation(-(OriginX*nWidth),-(OriginY*nHeight),&PanTransform);

  const float ScaleNormalX=cos(-DivisionAngle);
  const float ScaleNormalY=sin(-DivisionAngle);

  SPete_2dMatrix DirectionalScaleTransform;
  Pete_2dMatrix_SetToDirectionalScale(ScaleNormalX,ScaleNormalY,-1.0f,&DirectionalScaleTransform);

  SPete_2dMatrix WorldToScreen;
  Pete_2dMatrix_SetToTranslation(HalfWidth,HalfHeight,&WorldToScreen);

  int nCurrentDivision;
  for (nCurrentDivision=0; nCurrentDivision<nDivisionCount; nCurrentDivision+=1) {

    SPete_2dMatrix* pCurrentTransform=&pTransforms[nCurrentDivision];

    const bool bIsMirroredDivision=((nCurrentDivision&0x1)==0x1);

    float RotationAngle;
    if (!bIsMirroredDivision) {
      RotationAngle=DivisionAngle*nCurrentDivision;
    } else {
      RotationAngle=DivisionAngle*(nCurrentDivision-1);
    }

    SPete_2dMatrix RotationTransform;	
    Pete_2dMatrix_SetToRotation(RotationAngle,&RotationTransform);

    Pete_2dMatrix_SetToIdentity(pCurrentTransform);

    Pete_2dMatrix_Concatenate(pCurrentTransform,&ScreenToWorld,pCurrentTransform);

    Pete_2dMatrix_Concatenate(pCurrentTransform,&RotationTransform,pCurrentTransform);
		
    if (bIsMirroredDivision) {
      Pete_2dMatrix_Concatenate(pCurrentTransform,&DirectionalScaleTransform,pCurrentTransform);
    }
		
    Pete_2dMatrix_Concatenate(pCurrentTransform,&PanTransform,pCurrentTransform);

    Pete_2dMatrix_Concatenate(pCurrentTransform,&WorldToScreen,pCurrentTransform);

  }

}

void pix_kaleidoscope :: Pete_Kaleidoscope_Dev() {

  U32* pCurrentOutput=pOutput;

  SPete_2dVector GridA={1.0f,0.0f};
  SPete_2dVector GridB={0.0f,1.0f};

  const float Angle=m_OutputAnglePreIncrement;

  SPete_2dMatrix RotationTransform;
  Pete_2dMatrix_SetToRotation(Angle,&RotationTransform);

  Pete_2dMatrix_TransformVector(&GridA,&RotationTransform,&GridA);
  //	Pete_2dMatrix_TransformVector(&GridB,&RotationTransform,&GridB);

  const float GridMagA=20.0f;
  const float GridMagB=20.0f;

  const float RecipGridMagA=(1.0f/GridMagA);
  const float RecipGridMagB=(1.0f/GridMagB);

  const float Scale=2.0f;

  float Y;
  for (Y=0.0f; Y<nHeight; Y+=1.0f) {

    float X;
    for (X=0.0f; X<nWidth; X+=1.0f) {

      SPete_2dVector CurrentPos={X,Y};

      const float PosDotA=Pete_2dVector_DotProduct(&CurrentPos,&GridA);			
      const float PosDotB=Pete_2dVector_DotProduct(&CurrentPos,&GridB);			

      const float IntegralPosDotA=(floorf(PosDotA*RecipGridMagA)+0.5f)*GridMagA;
      const float IntegralPosDotB=(floorf(PosDotB*RecipGridMagB)+0.5f)*GridMagB;

      SPete_2dVector ScaledA;
      Pete_2dVector_Scale(&GridA,IntegralPosDotA,&ScaledA);

      SPete_2dVector ScaledB;
      Pete_2dVector_Scale(&GridB,IntegralPosDotB,&ScaledB);

      SPete_2dVector Centre;
      Pete_2dVector_Add(&ScaledA,&ScaledB,&Centre);

      SPete_2dVector SourcePos;
      Pete_2dVector_Subtract(&CurrentPos,&Centre,&SourcePos);

      Pete_2dVector_Scale(&SourcePos,Scale,&SourcePos);

      Pete_2dVector_Add(&SourcePos,&Centre,&SourcePos);

      int nSourceX = static_cast<int>(SourcePos.x);
      nSourceX=GetMirrored(nSourceX,nWidth);

      int nSourceY = static_cast<int>(SourcePos.y);
      nSourceY=GetMirrored(nSourceY,nHeight);

      U32* pCurrentSource=
	pSource+(nSourceY*nWidth)+nSourceX;

      *pCurrentOutput=*pCurrentSource;

      pCurrentOutput+=1;

    }
	
  }

}

inline int pix_kaleidoscope :: GetMirrored(int inValue,const int nMax) {

  const int nTwoMax=(nMax*2);

  int nOutValue=GetTiled(inValue,nTwoMax);

  if (nOutValue>=nMax) {
    nOutValue=((nTwoMax-1)-nOutValue);
  }

  return nOutValue;
}

void pix_kaleidoscope :: Pete_2dMatrix_SetToRotation(float Rotation,SPete_2dMatrix* poutResult) {

  const float CosRotation=cos(Rotation);
  const float SinRotation=sin(Rotation);
	
  poutResult->m[0][0]=CosRotation;
  poutResult->m[0][1]=-SinRotation;
  poutResult->m[0][2]=0.0f;

  poutResult->m[1][0]=SinRotation;
  poutResult->m[1][1]=CosRotation;
  poutResult->m[1][2]=0.0f;

  poutResult->m[2][0]=0.0f;
  poutResult->m[2][1]=0.0f;
  poutResult->m[2][2]=1.0f;

}
void pix_kaleidoscope :: Pete_2dMatrix_Concatenate(SPete_2dMatrix* pinFirst,SPete_2dMatrix* pinSecond,SPete_2dMatrix* poutResult) {

  SPete_2dMatrix TempResult;
	
  int nY=0;
  for (nY=0; nY<3; nY+=1) {

    int nX=0;
    for (nX=0; nX<3; nX+=1) {

      TempResult.m[nY][nX]=
	(pinSecond->m[nY][0]*pinFirst->m[0][nX])+
	(pinSecond->m[nY][1]*pinFirst->m[1][nX])+
	(pinSecond->m[nY][2]*pinFirst->m[2][nX]);

    }

  }

  *poutResult=TempResult;

}
void pix_kaleidoscope :: Pete_2dMatrix_SetToIdentity(SPete_2dMatrix* pMatrix) {

  pMatrix->m[0][0]=1.0f;
  pMatrix->m[0][1]=0.0f;
  pMatrix->m[0][2]=0.0f;

  pMatrix->m[1][0]=0.0f;
  pMatrix->m[1][1]=1.0f;
  pMatrix->m[1][2]=0.0f;

  pMatrix->m[2][0]=0.0f;
  pMatrix->m[2][1]=0.0f;
  pMatrix->m[2][2]=1.0f;

}

void pix_kaleidoscope :: Pete_2dMatrix_SetToDirectionalScale(float NormalX,float NormalY,float Scale,SPete_2dMatrix* poutResult) {

  const float ScaleMinusOne=(Scale-1.0f);

  poutResult->m[0][0]=1.0f+(NormalX*NormalX*ScaleMinusOne);
  poutResult->m[0][1]=(NormalX*NormalY*ScaleMinusOne);
  poutResult->m[0][2]=0.0f;

  poutResult->m[1][0]=(NormalY*NormalX*ScaleMinusOne);
  poutResult->m[1][1]=1.0f+(NormalY*NormalY*ScaleMinusOne);
  poutResult->m[1][2]=0.0f;

  poutResult->m[2][0]=0.0f;
  poutResult->m[2][1]=0.0f;
  poutResult->m[2][2]=1.0f;

}

void pix_kaleidoscope :: Pete_SimpleMirror_Render() {

  const float Width=nWidth;
  const float Height=nHeight;

  const float HalfWidth=(Width/2.0f);
  const float HalfHeight=(Height/2.0f);

  const float AngleRadians=m_Angle;

  const float NormalX=cos(AngleRadians);
  const float NormalY=sin(AngleRadians);

  const float MaxDist = sqrt((HalfWidth*HalfWidth)+(HalfHeight*HalfHeight));
  const float PlaneD=MaxDist*m_PlaneD;

  const bool bSimpleMirrorEverything=(m_DoSimpleMirrorAll>0.0f);

  const int nPixelsCount=(nWidth*nHeight);

  U32* pOutputStart=pOutput;
  U32* pOutputEnd=(pOutput+nPixelsCount);

  U32* pCurrentOutput=pOutputStart;
  U32* pCurrentSource=pSource;

  float CurrentY=-HalfHeight;
  while (pCurrentOutput<pOutputEnd) {

    U32* pOutputLineStart=pCurrentOutput;
    U32* pOutputLineEnd=(pOutputLineStart+nWidth);
		
    const float StartX=-HalfWidth;

    const float StartVDotNMinusD=
      ((StartX*NormalX)+
       (CurrentY*NormalY))-
      PlaneD;
		
    const float EndX=HalfWidth;

    float EndVDotNMinusD=
      ((EndX*NormalX)+
       (CurrentY*NormalY))-
      PlaneD;

    const float VDotNMinusDInc=
      (EndVDotNMinusD-StartVDotNMinusD)/nWidth;

    float VDotNMinusD=StartVDotNMinusD;

    float CurrentX=StartX;

    while (pCurrentOutput<pOutputLineEnd) {

      if ((VDotNMinusD>0.0f)&&(!bSimpleMirrorEverything)) {

	*pCurrentOutput=*pCurrentSource;

      } else {

	const float TwoVDotNMinusD=2.0f*VDotNMinusD;

	const float SourceX=CurrentX-(TwoVDotNMinusD*NormalX);
	const float SourceY=CurrentY-(TwoVDotNMinusD*NormalY);
		
	int nSourceX=static_cast<int>(SourceX+HalfWidth);
	int nSourceY=static_cast<int>(SourceY+HalfHeight);

	nSourceX=GetMirrored(nSourceX,nWidth);
	nSourceY=GetMirrored(nSourceY,nHeight);

	U32* pSimpleMirrorSource=
	  pSource+
	  (nSourceY*nWidth)+
	  nSourceX;

	*pCurrentOutput=*pSimpleMirrorSource;
			
      }

      CurrentX+=1.0f;
      VDotNMinusD+=VDotNMinusDInc;

      pCurrentOutput+=1;
      pCurrentSource+=1;

    }
		
    CurrentY+=1.0f;

  }
}

void pix_kaleidoscope :: Pete_2dMatrix_SetToTranslation(float TranslationX,float TranslationY,SPete_2dMatrix* poutResult) {

  poutResult->m[0][0]=1.0f;
  poutResult->m[0][1]=0.0f;
  poutResult->m[0][2]=TranslationX;

  poutResult->m[1][0]=0.0f;
  poutResult->m[1][1]=1.0f;
  poutResult->m[1][2]=TranslationY;

  poutResult->m[2][0]=0.0f;
  poutResult->m[2][1]=0.0f;
  poutResult->m[2][2]=1.0f;

}

void pix_kaleidoscope :: Pete_2dMatrix_TransformVector(SPete_2dVector* pinVector,SPete_2dMatrix* pinMatrix,SPete_2dVector* poutResult) {

  SPete_2dVector Result;	

  Result.x=
    (pinMatrix->m[0][0]*pinVector->x)+
    (pinMatrix->m[0][1]*pinVector->y)+
    (pinMatrix->m[0][2]);

  Result.y=
    (pinMatrix->m[1][0]*pinVector->x)+
    (pinMatrix->m[1][1]*pinVector->y)+
    (pinMatrix->m[1][2]);

  *poutResult=Result;

}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_kaleidoscope :: obj_setupCallback(t_class *classPtr)
{
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::sourceCtrCallback),
		  gensym("sourceCtr"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::outputCtrCallback),
		  gensym("outputCtr"), A_DEFFLOAT, A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::outputAngCallback),
		  gensym("outputAng"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::sourceAngCallback),
		  gensym("sourceAng"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::outputAngleCallback),
		  gensym("outputAngle"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::sourceAngleCallback),
		  gensym("sourceAngle"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::divCallback),
		  gensym("div"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::sapCallback),
		  gensym("sap"), A_DEFFLOAT, A_NULL);
  class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_kaleidoscope::rlpCallback),
		  gensym("rlp"), A_DEFFLOAT, A_NULL);
}
void pix_kaleidoscope :: divCallback(void *data, t_floatarg m_Divisions)
{
  GetMyClass(data)->m_Divisions=(m_Divisions);
  GetMyClass(data)->setPixModified();
}

void pix_kaleidoscope :: outputAngCallback(void *data, t_floatarg m_OutputAnglePreIncrement)
{
  GetMyClass(data)->m_OutputAnglePreIncrement=(m_OutputAnglePreIncrement);
  GetMyClass(data)->setPixModified();
}
void pix_kaleidoscope :: sourceAngCallback(void *data, t_floatarg m_SourceAnglePreIncrement)
{
  GetMyClass(data)->m_SourceAnglePreIncrement=(m_SourceAnglePreIncrement);  
  GetMyClass(data)->setPixModified();
}
void pix_kaleidoscope :: outputAngleCallback(void *data, t_floatarg m_OutputAnglePreIncrement)
{
  GetMyClass(data)->m_OutputAnglePreIncrement=(m_OutputAnglePreIncrement*deg2rad);
  GetMyClass(data)->setPixModified();
}
void pix_kaleidoscope :: sourceAngleCallback(void *data, t_floatarg m_SourceAnglePreIncrement)
{
  GetMyClass(data)->m_SourceAnglePreIncrement=(m_SourceAnglePreIncrement*deg2rad);
  GetMyClass(data)->setPixModified();
}
void pix_kaleidoscope :: sourceCtrCallback(void *data, t_floatarg m_SourceCentreX, t_floatarg m_SourceCentreY)
{
  GetMyClass(data)->m_SourceCentreX=(m_SourceCentreX);
  GetMyClass(data)->m_SourceCentreY=(m_SourceCentreY);  
  GetMyClass(data)->setPixModified();
}

void pix_kaleidoscope :: outputCtrCallback(void *data, t_floatarg m_OutputCentreX, t_floatarg m_OutputCentreY)
{
  GetMyClass(data)->m_OutputCentreX=(m_OutputCentreX);
  GetMyClass(data)->m_OutputCentreY=(m_OutputCentreY);
  GetMyClass(data)->setPixModified();
}
void pix_kaleidoscope :: rlpCallback(void *data, t_floatarg m_ReflectionLineProportion)
{
  GetMyClass(data)->m_ReflectionLineProportion=(m_ReflectionLineProportion);  
  GetMyClass(data)->setPixModified();
}

void pix_kaleidoscope :: sapCallback(void *data, t_floatarg m_SourceAngleProportion)
{
  GetMyClass(data)->m_SourceAngleProportion=(m_SourceAngleProportion);
  GetMyClass(data)->setPixModified();
}
