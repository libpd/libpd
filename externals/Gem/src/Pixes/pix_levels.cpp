////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// ported by tigital@mac.com
// ported from pete's_plugins (www.petewarden.com)
//
// Implementation file
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_levels.h"

CPPEXTERN_NEW(pix_levels);

/////////////////////////////////////////////////////////
//
// pix_levels
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_levels :: pix_levels()
{ 
    m_DoAuto = false;
    m_DoUniform = true;
    m_DoAllowInversion = true;
    
    m_UniformInputFloor = 0.0f;		// 0 to 255
    m_UniformInputCeiling = 255.0f;	// 0 to 255
    m_UniformOutputFloor = 0.0f;	// 0 to 255
    m_UniformOutputCeiling = 255.0f;	// 0 to 255
    
    m_RedInputFloor = 0.0f;		// 0 to 255
    m_RedInputCeiling = 255.0f;		// 0 to 255
    m_RedOutputFloor = 0.0f;		// 0 to 255
    m_RedOutputCeiling = 255.0f;	// 0 to 255
    
    m_GreenInputFloor = 0.0f;		// 0 to 255
    m_GreenInputCeiling = 255.0f;	// 0 to 255
    m_GreenOutputFloor = 0.0f;		// 0 to 255
    m_GreenOutputCeiling = 255.0f;	// 0 to 255
    
    m_BlueInputFloor = 0.0f;		// 0 to 255
    m_BlueInputCeiling = 255.0f;	// 0 to 255
    m_BlueOutputFloor = 0.0f;		// 0 to 255
    m_BlueOutputCeiling = 255.0f;	// 0 to 255
 
    m_AlphaInputFloor = 0.0f;		// 0 to 255
    m_AlphaInputCeiling = 255.0f;	// 0 to 255
    m_AlphaOutputFloor = 0.0f;		// 0 to 255
    m_AlphaOutputCeiling = 255.0f;	// 0 to 255  

    m_LowPercentile = 5.0f;		// 0 to 100
    m_HighPercentile = 95.0f;		// 0 to 100

  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("uniform"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("red"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("green"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("list"), gensym("blue"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("lowP"));
  inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("hiP"));
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_levels :: ~pix_levels()
{ }

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_levels :: processYUVImage(imageStruct &image)
{
    nWidth = image.xsize*image.csize/4;
    nHeight = image.ysize;
    
    pSource = reinterpret_cast<U32*>(image.data);

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.csize = image.csize;
    myImage.type  = image.type;
    myImage.format=image.format;
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    if(m_DoAuto)Pete_Levels_CalculateAutoLevels(GL_RGBA);
    Pete_Levels_SetupCFSettings();
    Pete_ChannelFunction_RenderYUV();

    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_levels :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize*image.csize/4;
    nHeight = image.ysize;

    pSource = reinterpret_cast<U32*>(image.data);

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.csize = image.csize;
    myImage.type  = image.type;
    myImage.format=image.format;
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    if(m_DoAuto)Pete_Levels_CalculateAutoLevels(GL_RGBA);
    Pete_Levels_SetupCFSettings();
    Pete_ChannelFunction_Render();

    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// various processing here
//
/////////////////////////////////////////////////////////
void pix_levels :: Pete_Levels_SetupCFSettings(int colour)
{
    const int cnFixedShift=16;
    const int cnFixedMult=(1<<cnFixedShift);
    const int cnFixedOne=1*cnFixedMult;
    if (m_DoUniform) {
      const int nInputLow=static_cast<int>(m_UniformInputFloor);
      int nInputDelta=static_cast<int>(m_UniformInputCeiling-m_UniformInputFloor);

      const int nOutputLow=static_cast<int>(m_UniformOutputFloor);
      int nOutputDelta=static_cast<int>(m_UniformOutputCeiling-m_UniformOutputFloor);
      // avoid the possibility of divide-by-zeros
      if (m_DoAllowInversion) {
	if (nInputDelta==0) nInputDelta=1;
	if (nOutputDelta==0)nOutputDelta=1;
	nInputDelta=clampFunc(nInputDelta,-255,255);
	nOutputDelta=clampFunc(nOutputDelta,-255,255);
      } else {
	nInputDelta=clampFunc(nInputDelta,1,255);
	nOutputDelta=clampFunc(nOutputDelta,1,255);
      }

      const int nRecipInputDelta=cnFixedOne/nInputDelta;

      int*const pRedTable=&(m_nRedTable[0]);
      int*const pGreenTable=&(m_nGreenTable[0]);
      int*const pBlueTable=&(m_nBlueTable[0]);
      int*const pAlphaTable=&(m_nAlphaTable[0]);

      int nCount;
      for (nCount=0; nCount<256; nCount+=1) {
	const int nSourceRed  =nCount;
	const int nSourceGreen=nCount;
	const int nSourceBlue =nCount;
	const int nSourceAlpha=nCount;

	const int nTempRed=  (((nSourceRed  -nInputLow)*256)*nRecipInputDelta)>>cnFixedShift;
	const int nTempGreen=(((nSourceGreen-nInputLow)*256)*nRecipInputDelta)>>cnFixedShift;
	const int nTempBlue= (((nSourceBlue -nInputLow)*256)*nRecipInputDelta)>>cnFixedShift;
	const int nTempAlpha=(((nSourceAlpha-nInputLow)*256)*nRecipInputDelta)>>cnFixedShift;

	int nOutputRed  =((nTempRed  *nOutputDelta)/256)+nOutputLow;
	int nOutputGreen=((nTempGreen*nOutputDelta)/256)+nOutputLow;
	int nOutputBlue =((nTempBlue *nOutputDelta)/256)+nOutputLow;
	int nOutputAlpha=((nTempAlpha*nOutputDelta)/256)+nOutputLow;

	nOutputRed  =clampFunc(nOutputRed  ,0,255);
	nOutputGreen=clampFunc(nOutputGreen,0,255);
	nOutputBlue =clampFunc(nOutputBlue ,0,255);
	nOutputAlpha=clampFunc(nOutputAlpha,0,255);

	pRedTable  [nCount]=(nOutputRed);
	pGreenTable[nCount]=(nOutputGreen);
	pBlueTable [nCount]=(nOutputBlue);
	pAlphaTable[nCount]=(nOutputAlpha);
      }
    } else { // !m_doUniform
	  const int nRedInputLow=static_cast<int>(m_RedInputFloor);
	  int nRedInputDelta=static_cast<int>(m_RedInputCeiling-m_RedInputFloor);
	  const int nRedOutputLow=static_cast<int>(m_RedOutputFloor);
	  int nRedOutputDelta=static_cast<int>(m_RedOutputCeiling-m_RedOutputFloor);

	  const int nGreenInputLow=static_cast<int>(m_GreenInputFloor);
	  int nGreenInputDelta=static_cast<int>(m_GreenInputCeiling-m_GreenInputFloor);
	  const int nGreenOutputLow=static_cast<int>(m_GreenOutputFloor);
	  int nGreenOutputDelta=static_cast<int>(m_GreenOutputCeiling-m_GreenOutputFloor);

	  const int nBlueInputLow=static_cast<int>(m_BlueInputFloor);
	  int nBlueInputDelta=static_cast<int>(m_BlueInputCeiling-m_BlueInputFloor);
	  const int nBlueOutputLow=static_cast<int>(m_BlueOutputFloor);
	  int nBlueOutputDelta=static_cast<int>(m_BlueOutputCeiling-m_BlueOutputFloor);

	  const int nAlphaInputLow=static_cast<int>(m_AlphaInputFloor);
	  int nAlphaInputDelta=static_cast<int>(m_AlphaInputCeiling-m_AlphaInputFloor);
	  const int nAlphaOutputLow=static_cast<int>(m_AlphaOutputFloor);
	  int nAlphaOutputDelta=static_cast<int>(m_AlphaOutputCeiling-m_AlphaOutputFloor);

	  // avoid the possibility of divide-by-zeros
	  if (m_DoAllowInversion) {
	    if (nRedInputDelta==0)   nRedInputDelta=1;
	    if (nRedOutputDelta==0)	 nRedOutputDelta=1;
	    if (nGreenInputDelta==0) nGreenInputDelta=1;
	    if (nGreenOutputDelta==0)nGreenOutputDelta=1;
	    if (nBlueInputDelta==0)	 nBlueInputDelta=1;
	    if (nBlueOutputDelta==0)  nBlueOutputDelta=1;
	    if (nAlphaInputDelta==0)	 nAlphaInputDelta=1;
	    if (nAlphaOutputDelta==0)  nAlphaOutputDelta=1;

	    nRedInputDelta=clampFunc(nRedInputDelta,-255,255);
	    nRedOutputDelta=clampFunc(nRedOutputDelta,-255,255);

	    nGreenInputDelta=clampFunc(nGreenInputDelta,-255,255);
	    nGreenOutputDelta=clampFunc(nGreenOutputDelta,-255,255);

	    nBlueInputDelta=clampFunc(nBlueInputDelta,-255,255);
	    nBlueOutputDelta=clampFunc(nBlueOutputDelta,-255,255);

	    nAlphaInputDelta=clampFunc(nAlphaInputDelta,-255,255);
	    nAlphaOutputDelta=clampFunc(nAlphaOutputDelta,-255,255);
	  } else {
	    nRedInputDelta=clampFunc(nRedInputDelta,1,255);
	    nRedOutputDelta=clampFunc(nRedOutputDelta,1,255);

	    nGreenInputDelta=clampFunc(nGreenInputDelta,1,255);
	    nGreenOutputDelta=clampFunc(nGreenOutputDelta,1,255);

	    nBlueInputDelta=clampFunc(nBlueInputDelta,1,255);
	    nBlueOutputDelta=clampFunc(nBlueOutputDelta,1,255);

	    nAlphaInputDelta=clampFunc(nAlphaInputDelta,1,255);
	    nAlphaOutputDelta=clampFunc(nAlphaOutputDelta,1,255);
	  }

	  const int nRedRecipInputDelta=cnFixedOne/nRedInputDelta;
	  const int nGreenRecipInputDelta=cnFixedOne/nGreenInputDelta;
	  const int nBlueRecipInputDelta=cnFixedOne/nBlueInputDelta;
	  const int nAlphaRecipInputDelta=cnFixedOne/nAlphaInputDelta;

	  int*const pRedTable=&(m_nRedTable[0]);
	  int*const pGreenTable=&(m_nGreenTable[0]);
	  int*const pBlueTable=&(m_nBlueTable[0]);
	  int*const pAlphaTable=&(m_nAlphaTable[0]);

	  int nCount;
	  for (nCount=0; nCount<256; nCount+=1) {
	    const int nSourceRed  =nCount;
	    const int nSourceGreen=nCount;
	    const int nSourceBlue =nCount;
	    const int nSourceAlpha=nCount;
	    
	    const int nTempRed=  (((nSourceRed  -nRedInputLow  )*256)*nRedRecipInputDelta  )>>cnFixedShift;
	    const int nTempGreen=(((nSourceGreen-nGreenInputLow)*256)*nGreenRecipInputDelta)>>cnFixedShift;
	    const int nTempBlue= (((nSourceBlue -nBlueInputLow )*256)*nBlueRecipInputDelta )>>cnFixedShift;
	    const int nTempAlpha=(((nSourceAlpha-nAlphaInputLow)*256)*nAlphaRecipInputDelta)>>cnFixedShift;
	    
	    int nOutputRed  =((nTempRed  *nRedOutputDelta  )/256)+nRedOutputLow;
	    int nOutputGreen=((nTempGreen*nGreenOutputDelta)/256)+nGreenOutputLow;
	    int nOutputBlue =((nTempBlue *nBlueOutputDelta )/256)+nBlueOutputLow;
	    int nOutputAlpha=((nTempAlpha*nAlphaOutputDelta)/256)+nAlphaOutputLow;
	    
	    nOutputRed  =clampFunc(nOutputRed  ,0,255);
	    nOutputGreen=clampFunc(nOutputGreen,0,255);
	    nOutputBlue =clampFunc(nOutputBlue ,0,255);
	    nOutputAlpha=clampFunc(nOutputAlpha,0,255);
	    
	    pRedTable  [nCount]=(nOutputRed);
	    pGreenTable[nCount]=(nOutputGreen);
	    pBlueTable [nCount]=(nOutputBlue);
	    pAlphaTable[nCount]=(nOutputAlpha);
	  }
    }
}

void pix_levels :: Pete_Levels_CalculateAutoLevels(int colour) {
	int	nRedHistogram[256];
	int	nGreenHistogram[256];
	int	nBlueHistogram[256];
	int	nAlphaHistogram[256];

	Pete_ZeroMemory(&nRedHistogram[0],256*sizeof(int));
	Pete_ZeroMemory(&nGreenHistogram[0],256*sizeof(int));
	Pete_ZeroMemory(&nBlueHistogram[0],256*sizeof(int));
	Pete_ZeroMemory(&nAlphaHistogram[0],256*sizeof(int));

	const int nNumPixels = nWidth*nHeight;
	
	U32* pCurrentSource=pSource;
	const U32* pSourceEnd=(pSource+nNumPixels);

	const int nSampleSpacing=8;

	while (pCurrentSource<pSourceEnd) {
		U32* pSourceLineStart=pCurrentSource;
		const U32* pSourceLineEnd=pCurrentSource+nWidth;

		while (pCurrentSource<pSourceLineEnd) {

			U32 SourceColour=*pCurrentSource;

			const int nSourceRed=(SourceColour>>SHIFT_RED)&0xff;
			const int nSourceGreen=(SourceColour>>SHIFT_GREEN)&0xff;
			const int nSourceBlue=(SourceColour>>SHIFT_BLUE)&0xff;
			const int nSourceAlpha=(SourceColour>>SHIFT_ALPHA)&0xff;

			switch(colour){
			default:
			  nRedHistogram[nSourceRed]+=1;
			  nGreenHistogram[nSourceGreen]+=1;
			  nBlueHistogram[nSourceBlue]+=1;
			  nAlphaHistogram[nSourceAlpha]+=1;
			  break;
			case (GL_LUMINANCE):
			  nRedHistogram  [nSourceRed] +=1;nRedHistogram  [nSourceGreen]+=1;
			  nRedHistogram  [nSourceBlue]+=1;nRedHistogram  [nSourceAlpha]+=1;
			  nGreenHistogram[nSourceRed] +=1;nGreenHistogram[nSourceGreen]+=1;
			  nGreenHistogram[nSourceBlue]+=1;nGreenHistogram[nSourceAlpha]+=1;
			  nBlueHistogram [nSourceRed] +=1;nBlueHistogram [nSourceGreen]+=1;
			  nBlueHistogram [nSourceBlue]+=1;nBlueHistogram [nSourceAlpha]+=1;
			  nAlphaHistogram[nSourceRed] +=1;nAlphaHistogram[nSourceGreen]+=1;
			  nAlphaHistogram[nSourceBlue]+=1;nAlphaHistogram[nSourceAlpha]+=1;
			  break;
			case (GL_YUV422_GEM):
			  nRedHistogram[nSourceRed]+=1; // U
			  nGreenHistogram[nSourceGreen]+=1; // Y0
			  nAlphaHistogram[nSourceGreen]+=1; // Y0
 			  nBlueHistogram[nSourceBlue]+=1; // V
			  nGreenHistogram[nSourceAlpha]+=1; // Y1
			  nAlphaHistogram[nSourceAlpha]+=1; // Y1
			  break;
			}


			pCurrentSource+=nSampleSpacing;
		}
		pCurrentSource=	pSourceLineStart+(nSampleSpacing*nWidth);
	}

	const int nSampleCount=	(nWidth/nSampleSpacing)*(nHeight/nSampleSpacing);

	const int nStartThreshold=static_cast<int>((m_LowPercentile*nSampleCount)/100.0f);
	const int nEndThreshold=static_cast<int>((m_HighPercentile*nSampleCount)/100.0f);

	int nCurrentRedTotal;
	int nCurrentGreenTotal;
	int nCurrentBlueTotal;
	int nCurrentAlphaTotal;

	int nCurrentSlot;

	nCurrentRedTotal=0;
	nCurrentSlot=0;
	while ((nCurrentRedTotal<nStartThreshold)&&(nCurrentSlot<256)) {
		nCurrentRedTotal+=nRedHistogram[nCurrentSlot];
		nCurrentSlot+=1;
	}

	const int nRedLow=(nCurrentSlot-1);

	nCurrentRedTotal=nSampleCount;
	nCurrentSlot=255;
	while ((nCurrentRedTotal>nEndThreshold)&&(nCurrentSlot>=0)) {
		nCurrentRedTotal-=nRedHistogram[nCurrentSlot];
		nCurrentSlot-=1;
	}

	const int nRedHigh=(nCurrentSlot+1);

	nCurrentGreenTotal=0;
	nCurrentSlot=0;
	while ((nCurrentGreenTotal<nStartThreshold)&&(nCurrentSlot<256)) {
		nCurrentGreenTotal+=nGreenHistogram[nCurrentSlot];
		nCurrentSlot+=1;
	}

	const int nGreenLow=(nCurrentSlot-1);

	nCurrentGreenTotal=nSampleCount;
	nCurrentSlot=255;
	while ((nCurrentGreenTotal>nEndThreshold)&&(nCurrentSlot>=0)) {
		nCurrentGreenTotal-=nGreenHistogram[nCurrentSlot];
		nCurrentSlot-=1;
	}

	const int nGreenHigh=(nCurrentSlot+1);

	nCurrentBlueTotal=0;
	nCurrentSlot=0;
	while ((nCurrentBlueTotal<nStartThreshold)&&(nCurrentSlot<256)) {
		nCurrentBlueTotal+=nBlueHistogram[nCurrentSlot];
		nCurrentSlot+=1;
	}

	const int nBlueLow=(nCurrentSlot-1);

	nCurrentBlueTotal=nSampleCount;
	nCurrentSlot=255;
	while ((nCurrentBlueTotal>nEndThreshold)&&(nCurrentSlot>=0)) {
		nCurrentBlueTotal-=nBlueHistogram[nCurrentSlot];
		nCurrentSlot-=1;
	}

	const int nBlueHigh=(nCurrentSlot+1);

	nCurrentAlphaTotal=0;
	nCurrentSlot=0;
	while ((nCurrentAlphaTotal<nStartThreshold)&&(nCurrentSlot<256)) {
		nCurrentAlphaTotal+=nAlphaHistogram[nCurrentSlot];
		nCurrentSlot+=1;
	}

	const int nAlphaLow=(nCurrentSlot-1);

	nCurrentAlphaTotal=nSampleCount;
	nCurrentSlot=255;
	while ((nCurrentAlphaTotal>nEndThreshold)&&(nCurrentSlot>=0)) {
		nCurrentAlphaTotal-=nAlphaHistogram[nCurrentSlot];
		nCurrentSlot-=1;
	}

	const int nAlphaHigh=(nCurrentSlot+1);

	m_RedInputFloor=static_cast<float>(nRedLow);
	if (nRedLow!=nRedHigh){
		m_RedInputCeiling=static_cast<float>(nRedHigh);
	} else if (nRedHigh<255) {
		m_RedInputCeiling=static_cast<float>(nRedHigh+1);
	} else {
		m_RedInputCeiling=static_cast<float>(nRedHigh-1);
	}

	m_GreenInputFloor=static_cast<float>(nGreenLow);
	if (nGreenLow!=nGreenHigh){
		m_GreenInputCeiling=static_cast<float>(nGreenHigh);
	} else if (nGreenHigh<255) {
		m_GreenInputCeiling=static_cast<float>(nGreenHigh+1);
	} else {
		m_GreenInputCeiling=static_cast<float>(nGreenHigh-1);
	}

	m_BlueInputFloor=static_cast<float>(nBlueLow);
	if (nBlueLow!=nBlueHigh){
		m_BlueInputCeiling=static_cast<float>(nBlueHigh);
	} else if (nBlueHigh<255) {
		m_BlueInputCeiling=static_cast<float>(nBlueHigh+1);
	} else {
		m_BlueInputCeiling=static_cast<float>(nBlueHigh-1);
	}

	m_AlphaInputFloor=static_cast<float>(nAlphaLow);
	if (nAlphaLow!=nAlphaHigh){
		m_AlphaInputCeiling=static_cast<float>(nAlphaHigh);
	} else if (nAlphaHigh<255) {
		m_AlphaInputCeiling=static_cast<float>(nAlphaHigh+1);
	} else {
		m_AlphaInputCeiling=static_cast<float>(nAlphaHigh-1);
	}

	int nLowLuminance = 
		((90 * nRedLow)+
		(115 * nGreenLow)+
		(51 * nBlueLow))/256;

	int nHighLuminance = 
		((90 * nRedHigh)+
		(115 * nGreenHigh)+
		(51 * nBlueHigh))/256;

	if (nStartThreshold<nEndThreshold) {
		if (nLowLuminance>=nHighLuminance) {
			nLowLuminance=clampFunc(nHighLuminance-1,0,255);
		}
	}
	m_UniformInputFloor=static_cast<float>(nLowLuminance);
	m_UniformInputCeiling=static_cast<float>(nHighLuminance);
}

void pix_levels :: Pete_ChannelFunction_Render() {

	const int*const pRedTable=m_nRedTable;
	const int*const pGreenTable=m_nGreenTable;
	const int*const pBlueTable=m_nBlueTable;
	const int*const pAlphaTable=m_nAlphaTable;

	const int nNumPixels = nWidth*nHeight;

	U32* pCurrentSource=pSource;
	U32* pCurrentOutput=pOutput;
	const U32* pSourceEnd=(pSource+nNumPixels);
	while (pCurrentSource!=pSourceEnd) {
		const U32 SourceColour=*pCurrentSource;
		const unsigned int nSourceRed=(SourceColour>>SHIFT_RED)&0xff;
		const unsigned int nSourceGreen=(SourceColour>>SHIFT_GREEN)&0xff;
		const unsigned int nSourceBlue=(SourceColour>>SHIFT_BLUE)&0xff;
		//		const unsigned int nSourceAlpha=(SourceColour&(((U32)0xff)<<SHIFT_ALPHA));
		const unsigned int nSourceAlpha=(SourceColour>>SHIFT_ALPHA)&0xff;
		//const unsigned int nSourceAlpha0=(SourceColour&(((U32)0xff)<<SHIFT_ALPHA));


		const int nOutputRed=pRedTable[nSourceRed];
		const int nOutputGreen=pGreenTable[nSourceGreen];
		const int nOutputBlue=pBlueTable[nSourceBlue];
		const int nOutputAlpha=pAlphaTable[nSourceAlpha];
		/*
		post("IN  %d %d %d %d", (nSourceRed&0xff), (nSourceGreen&0xff), (nSourceBlue&0xff), (nSourceAlpha&0xff));

		post("OUT %d %d %d %d", (nOutputRed&0xff), (nOutputGreen&0xff), (nOutputBlue&0xff), (nOutputAlpha&0xff));
		*/
		const U32 OutputColour=
		  ((nOutputRed&0xff)<<SHIFT_RED)|
		  ((nOutputGreen&0xff)<<SHIFT_GREEN)|
		  ((nOutputBlue&0xff)<<SHIFT_BLUE)|
		  ((nOutputAlpha&0xff)<<SHIFT_ALPHA);

		*pCurrentOutput=OutputColour;
		pCurrentSource+=1;
		pCurrentOutput+=1;
	}
}


void pix_levels :: Pete_ChannelFunction_RenderYUV() {

    const int*const pRedTable=m_nRedTable;
    const int*const pGreenTable=m_nGreenTable;
    const int*const pBlueTable=m_nBlueTable;
    const int*const pAlphaTable=m_nAlphaTable;

    const int nNumPixels = nWidth*nHeight;

    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
    const U32* pSourceEnd=(pSource+nNumPixels);
    while (pCurrentSource!=pSourceEnd) {
        const U32 SourceColour=*pCurrentSource;
        const unsigned int nSourceU=(SourceColour>>SHIFT_U)&0xff;
        const unsigned int nSourceY1=(SourceColour>>SHIFT_Y1)&0xff;
        const unsigned int nSourceV=(SourceColour>>SHIFT_V)&0xff;
        //		const unsigned int nSourceAlpha=(SourceColour&(((U32)0xff)<<SHIFT_ALPHA));
        const unsigned int nSourceY2=(SourceColour>>SHIFT_Y2)&0xff;
        //const unsigned int nSourceAlpha0=(SourceColour&(((U32)0xff)<<SHIFT_ALPHA));


        const int nOutputY1=pRedTable[nSourceY1];
        const int nOutputY2=pBlueTable[nSourceY2];
        const int nOutputU=pAlphaTable[nSourceU];
        const int nOutputV=pGreenTable[nSourceV];
 //       const int nOutputBlue=pBlueTable[nSourceBlue];
 //       const int nOutputAlpha=pAlphaTable[nSourceAlpha];
        /*
         post("IN  %d %d %d %d", (nSourceRed&0xff), (nSourceGreen&0xff), (nSourceBlue&0xff), (nSourceAlpha&0xff));

         post("OUT %d %d %d %d", (nOutputRed&0xff), (nOutputGreen&0xff), (nOutputBlue&0xff), (nOutputAlpha&0xff));
         */
        const U32 OutputColour=
            ((nOutputU&0xff)<<SHIFT_U)| //Y0
            ((nOutputY1&0xff)<<SHIFT_Y1)| //Y1
            ((nOutputV&0xff)<<SHIFT_V)| //V
            ((nOutputY2&0xff)<<SHIFT_Y2); //U

        *pCurrentOutput=OutputColour;
        pCurrentSource+=1;
        pCurrentOutput+=1;
    }
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_levels :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::autoCallback),
		  gensym("auto"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::uniCallback),
		  gensym("uni"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::invCallback),
		  gensym("inv"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::uniformCallback),
		  gensym("uniform"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::redCallback),
		  gensym("red"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::greenCallback),
		  gensym("green"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::blueCallback),
		  gensym("blue"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::lowPCallback),
		  gensym("lowP"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_levels::hiPCallback),
		  gensym("hiP"), A_DEFFLOAT, A_NULL);
}

void pix_levels :: autoCallback(void *data, t_floatarg m_DoAuto)
{
  GetMyClass(data)->m_DoAuto=(m_DoAuto>0.);
  GetMyClass(data)->setPixModified();
}

void pix_levels :: uniCallback(void *data, t_floatarg m_DoUniform)
{
  GetMyClass(data)->m_DoUniform=(m_DoUniform>0.);
  GetMyClass(data)->setPixModified();
}
void pix_levels :: invCallback(void *data, t_floatarg m_DoAllowInversion)
{
  GetMyClass(data)->m_DoAllowInversion=(m_DoAllowInversion>0.);
  GetMyClass(data)->setPixModified();
}

void pix_levels :: lowPCallback(void *data, t_floatarg m_LowPercentile)
{
  GetMyClass(data)->m_LowPercentile=(m_LowPercentile);
  GetMyClass(data)->setPixModified();
}
void pix_levels :: hiPCallback(void *data, t_floatarg m_HighPercentile)
{
  GetMyClass(data)->m_HighPercentile=(m_HighPercentile);  
  GetMyClass(data)->setPixModified();
}

void pix_levels :: uniformCallback(void *data, t_floatarg m_UniformInputFloor, t_floatarg m_UniformInputCeiling, t_floatarg m_UniformOutputFloor, t_floatarg m_UniformOutputCeiling)
{
  GetMyClass(data)->m_UniformInputFloor=(m_UniformInputFloor*255.);
  GetMyClass(data)->m_UniformInputCeiling=(m_UniformInputCeiling*255.);
  GetMyClass(data)->m_UniformOutputFloor=(m_UniformOutputFloor*255.);
  GetMyClass(data)->m_UniformOutputCeiling=(m_UniformOutputCeiling*255.);  
  GetMyClass(data)->setPixModified();
}
void pix_levels :: redCallback(void *data, t_floatarg m_RedInputFloor, t_floatarg m_RedInputCeiling, t_floatarg m_RedOutputFloor, t_floatarg m_RedOutputCeiling)
{
  GetMyClass(data)->m_RedInputFloor=(m_RedInputFloor*255.);
  GetMyClass(data)->m_RedInputCeiling=(m_RedInputCeiling*255.);
  GetMyClass(data)->m_RedOutputFloor=(m_RedOutputFloor*255.);
  GetMyClass(data)->m_RedOutputCeiling=(m_RedOutputCeiling*255.);  
  GetMyClass(data)->setPixModified();
}
void pix_levels :: greenCallback(void *data, t_floatarg m_GreenInputFloor, t_floatarg m_GreenInputCeiling, t_floatarg m_GreenOutputFloor, t_floatarg m_GreenOutputCeiling)
{
  GetMyClass(data)->m_GreenInputFloor=(m_GreenInputFloor*255.);
  GetMyClass(data)->m_GreenInputCeiling=(m_GreenInputCeiling*255.);
  GetMyClass(data)->m_GreenOutputFloor=(m_GreenOutputFloor*255.);
  GetMyClass(data)->m_GreenOutputCeiling=(m_GreenOutputCeiling*255.);  
  GetMyClass(data)->setPixModified();
}
void pix_levels :: blueCallback(void *data, t_floatarg m_BlueInputFloor, t_floatarg m_BlueInputCeiling, t_floatarg m_BlueOutputFloor, t_floatarg m_BlueOutputCeiling)
{
  GetMyClass(data)->m_BlueInputFloor=(m_BlueInputFloor*255.);
  GetMyClass(data)->m_BlueInputCeiling=(m_BlueInputCeiling*255.);
  GetMyClass(data)->m_BlueOutputFloor=(m_BlueOutputFloor*255.);
  GetMyClass(data)->m_BlueOutputCeiling=(m_BlueOutputCeiling*255.);  
  GetMyClass(data)->setPixModified();
}
