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
#include "pix_lumaoffset.h"
	
CPPEXTERN_NEW(pix_lumaoffset);

/////////////////////////////////////////////////////////
//
// pix_lumaoffset
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_lumaoffset :: pix_lumaoffset()
{ 
    m_OffsetScale = 12.0f; 	// -127 to 127
    m_LineGap = 1.2f;		// 0 to 32
    m_DoFilledLines = false;	// 0 or 1
    m_DoSmoothFill = false;	// 0 or 1

    init =0;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("offset"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("gap"));

    hPreviousLineHeights_size=0;
    hPreviousLineHeights=NULL;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_lumaoffset :: ~pix_lumaoffset()
{ 
    
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_lumaoffset :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    const int nNumPixels=nWidth*nHeight;

    const int nOffsetScale=static_cast<int>(m_OffsetScale);
    const int nLineGap=static_cast<int>(m_LineGap);

    Pete_ZeroMemory(pOutput,(nNumPixels*sizeof(U32)));
	
    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
	
    U32* pSourceEnd=(pSource+nNumPixels);
    U32* pOutputEnd=(pOutput+nNumPixels);

    if (!m_DoFilledLines) {
	while (pCurrentSource<pSourceEnd) {
	    const U32* pSourceLineEnd=pCurrentSource+nWidth;

	    while (pCurrentSource!=pSourceLineEnd) {
		const U32 SourceColour=*pCurrentSource;
		int nLuma=GetLuminance(SourceColour);
		nLuma-=(128*255);
				
		const int nOffset=(nLuma*nOffsetScale)>>16;
		U32*const pOffsetOutput=
			    pCurrentOutput+(nOffset*nWidth);

		if ((pOffsetOutput<pOutputEnd)&& (pOffsetOutput>=pOutput)) {

		    *pOffsetOutput=SourceColour;
		}

		pCurrentSource+=1;
		pCurrentOutput+=1;
	    }
	    pCurrentSource+=(nWidth*nLineGap);
	    pCurrentOutput+=(nWidth*nLineGap);
	}
    } else {
      if (hPreviousLineHeights==NULL || hPreviousLineHeights_size<static_cast<int>(nWidth*sizeof(U32*)) ){
	free(hPreviousLineHeights);
	hPreviousLineHeights_size=nWidth*sizeof(U32*);
	hPreviousLineHeights=malloc(hPreviousLineHeights_size);	
      }
      U32** ppPreviousLineHeights=static_cast<U32**>(Pete_LockHandle(hPreviousLineHeights));
	if (ppPreviousLineHeights==NULL)return;

	Pete_ZeroMemory(ppPreviousLineHeights,(nWidth*sizeof(U32*)));
	//int nCurrentY=0;
	while (pCurrentSource<pSourceEnd) {
	    const U32* pSourceLineEnd=pCurrentSource+nWidth;
	    U32** ppCurrentLineHeight=ppPreviousLineHeights;

	    if (m_DoSmoothFill) {
		while (pCurrentSource!=pSourceLineEnd) {
		    const U32 SourceColour=*pCurrentSource;
		    const int nSourceRed=(SourceColour>>SHIFT_RED)&0xff;
		    const int nSourceGreen=(SourceColour>>SHIFT_GREEN)&0xff;
		    const int nSourceBlue=(SourceColour>>SHIFT_BLUE)&0xff;
		    const int nSourceAlpha=(SourceColour>>SHIFT_ALPHA)&0xff;

		    int nLuma=GetLuminance(SourceColour);
		    nLuma-=(128*255);
                    const int currentLineNumber=pCurrentOutput-pOutput;
		    const int nOffset0=(nLuma*nOffsetScale)>>16;
                    const int nOffset = (nOffset0>(nHeight-1))?(nHeight-1):
                                        (nOffset0>(currentLineNumber))?(currentLineNumber):nOffset0;

		    U32* pOffsetOutputStart=
			    pCurrentOutput+(nOffset*nWidth);

		    U32* pOffsetOutput=pOffsetOutputStart;
		    U32* pPreviousOffsetOutput=*ppCurrentLineHeight;

		    int nDestRed;
		    int nDestGreen;
		    int nDestBlue;
		    int nDestAlpha;
		    int nDestDistance;
		    if (pPreviousOffsetOutput==NULL) {
			nDestRed=0;
			nDestGreen=0;
			nDestBlue=0;
			nDestAlpha=255;
			nDestDistance=10000;
		    } else {
			const U32 DestColour=*pPreviousOffsetOutput;
			nDestRed=(DestColour>>SHIFT_RED)&0xff;
			nDestGreen=(DestColour>>SHIFT_GREEN)&0xff;
			nDestBlue=(DestColour>>SHIFT_BLUE)&0xff;
			nDestAlpha=(DestColour>>SHIFT_ALPHA)&0xff;
			nDestDistance=(pOffsetOutput-pPreviousOffsetOutput)/nWidth;
			if (nDestDistance==0)nDestDistance=1;
		    }

		    const int nDeltaRed=(nDestRed-nSourceRed);
		    const int nDeltaGreen=(nDestGreen-nSourceGreen);
		    const int nDeltaBlue=(nDestBlue-nSourceBlue);
		    const int nDeltaAlpha=(nDestAlpha-nSourceAlpha);
		    
		    const int nIncRed=(nDeltaRed/nDestDistance);
		    const int nIncGreen=(nDeltaGreen/nDestDistance);
		    const int nIncBlue=(nDeltaBlue/nDestDistance);
		    const int nIncAlpha=(nDeltaAlpha/nDestDistance);

		    int nCurrentRed=nSourceRed;
		    int nCurrentGreen=nSourceGreen;
		    int nCurrentBlue=nSourceBlue;
		    int nCurrentAlpha=nSourceAlpha;

		    while ((pOffsetOutput<pOutputEnd)&&
				(pOffsetOutput>=pOutput)&&
				(pOffsetOutput>pPreviousOffsetOutput)) {
			U32 CurrentColour=
					    (nCurrentRed<<SHIFT_RED)|
					    (nCurrentGreen<<SHIFT_GREEN)|
					    (nCurrentBlue<<SHIFT_BLUE)|
					    (nCurrentAlpha<<SHIFT_ALPHA);

			*pOffsetOutput=CurrentColour;

			nCurrentRed+=nIncRed;
			nCurrentGreen+=nIncGreen;
			nCurrentBlue+=nIncBlue;
			nCurrentAlpha+=nIncAlpha;
					
			pOffsetOutput-=nWidth;
		    }

		    *ppCurrentLineHeight=pOffsetOutputStart;

		    pCurrentSource+=1;
		    pCurrentOutput+=1;
		    ppCurrentLineHeight+=1;
		}
	    } else {
		while (pCurrentSource!=pSourceLineEnd) {
		    const U32 SourceColour=*pCurrentSource;
		    int nLuma=GetLuminance(SourceColour);
		    nLuma-=(128*255);
		    const int nOffset=(nLuma*nOffsetScale)>>16;
		    U32* pOffsetOutputStart=pCurrentOutput+(nOffset*nWidth);
		    U32* pOffsetOutput=pOffsetOutputStart;
		    U32* pPreviousOffsetOutput=*ppCurrentLineHeight;

		    while ((pOffsetOutput<pOutputEnd)&&
				(pOffsetOutput>=pOutput)&&
				(pOffsetOutput>pPreviousOffsetOutput)) {
			*pOffsetOutput=SourceColour;
			pOffsetOutput-=nWidth;
		    }

		    *ppCurrentLineHeight=pOffsetOutputStart;

		    pCurrentSource+=1;
		    pCurrentOutput+=1;
		    ppCurrentLineHeight+=1;
		}
	    }
	    pCurrentSource+=(nWidth*nLineGap);
	    pCurrentOutput+=(nWidth*nLineGap);
	}

	U32* pFinalLineStart=pOutputEnd-nWidth;
	U32* pFinalLineEnd=pOutputEnd;
	U32** ppCurrentLineHeight=ppPreviousLineHeights;
	U32* pCurrentOutput=pFinalLineStart;

	while (pCurrentOutput<pFinalLineEnd) {
	    U32* pPreviousOffsetOutput=*ppCurrentLineHeight;
	    const U32 SourceColour=*pPreviousOffsetOutput;
	    U32* pOffsetOutput=pCurrentOutput;

	    while ((pOffsetOutput<pOutputEnd)&&
		    (pOffsetOutput>=pOutput)&&
		    (pOffsetOutput>pPreviousOffsetOutput)) {

		*pOffsetOutput=SourceColour;
		pOffsetOutput-=nWidth;
	    }
	    pCurrentOutput+=1;
	    ppCurrentLineHeight+=1;
	}
    }
    image.data = myImage.data;
}
void pix_lumaoffset :: processGrayImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	init = 1;
    }
    U8* pSource = static_cast<U8*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    U8* pOutput = static_cast<U8*>(myImage.data);

    const int nNumPixels=nWidth*nHeight;

    const int nOffsetScale=static_cast<int>(m_OffsetScale);
    const int nLineGap=static_cast<int>(m_LineGap);

    Pete_ZeroMemory(pOutput,(nNumPixels*sizeof(U8)));
	
    U8* pCurrentSource=pSource;
    U8* pCurrentOutput=pOutput;
	
    U8* pSourceEnd=(pSource+nNumPixels);
    U8* pOutputEnd=(pOutput+nNumPixels);

    if (!m_DoFilledLines) {
	while (pCurrentSource<pSourceEnd) {
	    const U8* pSourceLineEnd=pCurrentSource+nWidth;

	    while (pCurrentSource!=pSourceLineEnd) {
		const U8 SourceColour=*pCurrentSource;
		int nLuma=(SourceColour<<8);
		nLuma-=(128*255);
		const int nOffset=(nLuma*nOffsetScale)>>16;
		U8*const pOffsetOutput=
			    pCurrentOutput+(nOffset*nWidth);

		if ((pOffsetOutput<pOutputEnd)&& (pOffsetOutput>=pOutput)) {

		    *pOffsetOutput=SourceColour;
		}

		pCurrentSource+=1;
		pCurrentOutput+=1;
	    }
	    pCurrentSource+=(nWidth*nLineGap);
	    pCurrentOutput+=(nWidth*nLineGap);
	}
    } else {
      if (hPreviousLineHeights==NULL || hPreviousLineHeights_size<static_cast<int>(nWidth*sizeof(U8*)) ){
	free(hPreviousLineHeights);
	hPreviousLineHeights_size=nWidth*sizeof(U8*);
	hPreviousLineHeights=malloc(hPreviousLineHeights_size);	
      }
      U8** ppPreviousLineHeights=static_cast<U8**>(Pete_LockHandle(hPreviousLineHeights));
	if (ppPreviousLineHeights==NULL)return;

	Pete_ZeroMemory(ppPreviousLineHeights,(nWidth*sizeof(U8*)));

	//int nCurrentY=0;
	while (pCurrentSource<pSourceEnd) {
	    const U8* pSourceLineEnd=pCurrentSource+nWidth;
	    U8** ppCurrentLineHeight=ppPreviousLineHeights;

	    if (m_DoSmoothFill) {
		while (pCurrentSource!=pSourceLineEnd) {
		    const U8 SourceColour=*pCurrentSource;

		    int nLuma=(SourceColour<<8);
		    nLuma-=(128*255);
                    const int currentLineNumber=pCurrentOutput-pOutput;
		    const int nOffset0=(nLuma*nOffsetScale)>>16;
                    const int nOffset = (nOffset0>(nHeight-1))?(nHeight-1):
                                        (nOffset0>(currentLineNumber))?(currentLineNumber):nOffset0;
		    //const int nOffset=(nLuma*nOffsetScale)>>16;
                    
		    U8* pOffsetOutputStart=
			    pCurrentOutput+(nOffset*nWidth);

		    U8* pOffsetOutput=pOffsetOutputStart;
		    U8* pPreviousOffsetOutput=*ppCurrentLineHeight;

		    int nDestGray;
		    int nDestDistance;
		    if (pPreviousOffsetOutput==NULL) {
			nDestGray=0;
			nDestDistance=10000;
		    } else {
			const U8 DestColour=*pPreviousOffsetOutput;
			nDestGray=DestColour;
			nDestDistance=(pOffsetOutput-pPreviousOffsetOutput)/nWidth;
			if(nDestDistance==0)nDestDistance=1;
		    }

		    const int nDeltaGray=(nDestGray-SourceColour);
		    const int nIncGray=(nDeltaGray/nDestDistance);
		    int nCurrentGray=SourceColour;

		    while ((pOffsetOutput<pOutputEnd)&&
				(pOffsetOutput>=pOutput)&&
				(pOffsetOutput>pPreviousOffsetOutput)) {
			*pOffsetOutput=nCurrentGray;
			nCurrentGray+=nIncGray;
					
			pOffsetOutput-=nWidth;
		    }

		    *ppCurrentLineHeight=pOffsetOutputStart;

		    pCurrentSource+=1;
		    pCurrentOutput+=1;
		    ppCurrentLineHeight+=1;
		}
	    } else {
		while (pCurrentSource!=pSourceLineEnd) {
		    const U8 SourceColour=*pCurrentSource;
		    int nLuma=(SourceColour<<8);
		    nLuma-=(128*255);
		    const int nOffset=(nLuma*nOffsetScale)>>16;
		    U8* pOffsetOutputStart=pCurrentOutput+(nOffset*nWidth);
		    U8* pOffsetOutput=pOffsetOutputStart;
		    U8* pPreviousOffsetOutput=*ppCurrentLineHeight;

		    while ((pOffsetOutput<pOutputEnd)&&
				(pOffsetOutput>=pOutput)&&
				(pOffsetOutput>pPreviousOffsetOutput)) {
			*pOffsetOutput=SourceColour;
			pOffsetOutput-=nWidth;
		    }

		    *ppCurrentLineHeight=pOffsetOutputStart;

		    pCurrentSource+=1;
		    pCurrentOutput+=1;
		    ppCurrentLineHeight+=1;
		}
	    }
	    pCurrentSource+=(nWidth*nLineGap);
	    pCurrentOutput+=(nWidth*nLineGap);
	}

	U8* pFinalLineStart=pOutputEnd-nWidth;
	U8* pFinalLineEnd=pOutputEnd;
	U8** ppCurrentLineHeight=ppPreviousLineHeights;
	U8* pCurrentOutput=pFinalLineStart;

	while (pCurrentOutput<pFinalLineEnd) {
	    U8* pPreviousOffsetOutput=*ppCurrentLineHeight;
	    const U8 SourceColour=*pPreviousOffsetOutput;
	    U8* pOffsetOutput=pCurrentOutput;

	    while ((pOffsetOutput<pOutputEnd)&&
		    (pOffsetOutput>=pOutput)&&
		    (pOffsetOutput>pPreviousOffsetOutput)) {

		*pOffsetOutput=SourceColour;
		pOffsetOutput-=nWidth;
	    }
	    pCurrentOutput+=1;
	    ppCurrentLineHeight+=1;
	}
    }
    image.data = myImage.data;
}
/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_lumaoffset :: processYUVImage(imageStruct &image)
{
	int nSourceU, nSourceY1, nSourceV, nSourceY2;
	
    nWidth = image.xsize/2;
    nHeight = image.ysize;
    if (!init) {
		init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    myImage.setBlack();
    pOutput = reinterpret_cast<U32*>(myImage.data);

    const int nNumPixels=nWidth*nHeight;

    const int nOffsetScale=static_cast<int>(m_OffsetScale);
    const int nLineGap=static_cast<int>(m_LineGap);

    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
	
    U32* pSourceEnd = pSource+nNumPixels;
    U32* pOutputEnd = pOutput+nNumPixels;

    if (!m_DoFilledLines) {
		while (pCurrentSource < pSourceEnd) {
			const U32* pSourceLineEnd = pCurrentSource + nWidth;

			while ( pCurrentSource != pSourceLineEnd) {
			  const U32 SourceColour=*pCurrentSource;
			  //nSourceY1 = ((SourceColour&(0xff<<16))>>16)<<8;
			  nSourceY1 = ((SourceColour>>SHIFT_Y1)&0xff)<<8;
			  nSourceY1 -= 32640;	// 128*255
			  nSourceY2 = ((SourceColour>>SHIFT_Y2)&0xff)<<8;
			  //nSourceY2 = ((SourceColour&(0xff<<0))>>0)<<8;
			  nSourceY2 -= 32640;
				
				const int nOffset1=(nSourceY1*nOffsetScale)>>16;
				const int nOffset2=(nSourceY2*nOffsetScale)>>16;
				
				U32* pOffsetOutput1=(pCurrentOutput+(nOffset1*nWidth));

				if ((pOffsetOutput1<pOutputEnd) && static_cast<int>(pOffsetOutput1>=pOutput)) {
					if (!(abs(nOffset1)>>1)){
						*pOffsetOutput1  = *pOffsetOutput1 & 0x000000ff;
						*pOffsetOutput1 |= SourceColour & 0xffffff00;
					}else{
						*pOffsetOutput1  = *pOffsetOutput1 & 0x00ff0000;
						*pOffsetOutput1 |= SourceColour & 0xff00ffff;
					}
				}
				
				U32* pOffsetOutput2=(pCurrentOutput)+(nOffset2*nWidth);

				if ((pOffsetOutput2<pOutputEnd) && static_cast<int>(pOffsetOutput2>=pOutput)) {
					if (!(abs(nOffset2)>>1)){
						*pOffsetOutput2  = *pOffsetOutput2 & 0x00ff0000;
						*pOffsetOutput2 |= SourceColour & 0xff00ffff;
					}else{
						*pOffsetOutput2  = *pOffsetOutput2 & 0x000000ff;
						*pOffsetOutput2 |= SourceColour & 0xffffff00;
					}
				}

				pCurrentSource+=1;
				pCurrentOutput+=1;
			}
			pCurrentSource+=(nWidth*nLineGap);
			pCurrentOutput+=(nWidth*nLineGap);
		}
    } else {
		if (hPreviousLineHeights==NULL || hPreviousLineHeights_size<static_cast<int>(nWidth*sizeof(U32*)) ){
			free(hPreviousLineHeights);
			hPreviousLineHeights_size=nWidth*sizeof(U32*);
			hPreviousLineHeights=malloc(hPreviousLineHeights_size);	
		}
		U32** ppPreviousLineHeights=static_cast<U32**>(Pete_LockHandle(hPreviousLineHeights));
		if (ppPreviousLineHeights==NULL)return;

		Pete_ZeroMemory(ppPreviousLineHeights,(nWidth*sizeof(U32*)));
		
		if (hPreviousLineHeights2 ==NULL || hPreviousLineHeights_size<static_cast<int>(nWidth*sizeof(U32*)) ){
			free(hPreviousLineHeights2);
			hPreviousLineHeights_size=nWidth*sizeof(U32*);
			hPreviousLineHeights2=malloc(hPreviousLineHeights_size);	
		}
		U32** ppPreviousLineHeights2=static_cast<U32**>(Pete_LockHandle(hPreviousLineHeights2));
		if (ppPreviousLineHeights2 ==NULL)return;

		Pete_ZeroMemory(ppPreviousLineHeights2,(nWidth*sizeof(U32*)));

		//int nCurrentY=0;
		while (pCurrentSource<pSourceEnd) {
			const U32* pSourceLineEnd=pCurrentSource+nWidth;
			U32** ppCurrentLineHeight=ppPreviousLineHeights;
			//U32** ppCurrentLineHeight2 = ppPreviousLineHeights2;


                        if (m_DoSmoothFill) {
                            while (pCurrentSource!=pSourceLineEnd) {
                                const U32 SourceColour=*pCurrentSource;
                                nSourceU = ((SourceColour&(0xff<<24))>>24);
                                nSourceY1 = ((SourceColour&(0xff<<16))>>16)<<8;
                                nSourceY1 -= 32640; 
                                nSourceV = ((SourceColour&(0xff<<8))>>8);
                                nSourceY2 = ((SourceColour&(0xff<<0))>>0)<<8;
                                nSourceY2 -= 32640;
                                const int currentLineNumber=pCurrentOutput-pOutput;
                                const int nOffset0=(nSourceY1*nOffsetScale)>>16;
                                const int nOffset1 = (nOffset0>(nHeight-1))?(nHeight-1):
                                        (nOffset0>(currentLineNumber))?(currentLineNumber):nOffset0;
//                                const int nOffset1=(nSourceY1*nOffsetScale)>>16;

                              //  const int nOffset2=(nSourceY2*nOffsetScale)>>16;
                                
                                //this doesn't fit with the rest of the Y handling but it works
                                nSourceY1 = ((SourceColour&(0xff<<16))>>16);
                                nSourceY2 = ((SourceColour&(0xff<<0))>>0);
                                

                                U32* pOffsetOutputStart1 = pCurrentOutput+(nOffset1*nWidth);

                                U32* pOffsetOutput1 = pOffsetOutputStart1;
                                U32* pPreviousOffsetOutput1 = *ppCurrentLineHeight;


                                int nDestU;
                                int nDestY1;
                                int nDestV;
                                int nDestY2;
                                int nDestDistance;
                                if (pPreviousOffsetOutput1==NULL) {
                                    nDestU = 128;
                                    nDestY1 = 0;
                                    nDestV = 128;
                                    nDestY2 = 0;
                                    nDestDistance = 10000;
                                } else {
                                    const U32 DestColour=*pPreviousOffsetOutput1;

                                    nDestU = ((DestColour&(0xff<<24))>>24);
                                    nDestY1 = ((DestColour&(0xff<<16))>>16);
                                    nDestV = ((DestColour&(0xff<<8))>>8);
                                    nDestY2 = ((DestColour&(0xff<<0))>>0);

                                    nDestDistance=(pOffsetOutput1 - pPreviousOffsetOutput1)/nWidth;
                                    if (nDestDistance==0)nDestDistance=1;
                                }
                                

                                const int nDeltaU= (nDestU-nSourceU);
                                const int nDeltaY1=(nDestY1-nSourceY1);
                                const int nDeltaV=(nDestV-nSourceV);
                                const int nDeltaY2=(nDestY2-nSourceY2);

                                const int nIncU =(nDeltaU/nDestDistance);
                                const int nIncY1=(nDeltaY1/nDestDistance);
                                const int nIncV = (nDeltaV/nDestDistance);
                                const int nIncY2= (nDeltaY2/nDestDistance);

                                int nCurrentU = nSourceU;
                                int nCurrentY1 = nSourceY1;
                                int nCurrentV = nSourceV;
                                int nCurrentY2 = nSourceY2;

                                while ((pOffsetOutput1 < pOutputEnd)&&(pOffsetOutput1 >=pOutput)&&
                                       (pOffsetOutput1 > pPreviousOffsetOutput1)) {

                                    //the & seems needed to assure no stry values
                                    U32 CurrentColour1=
                                    ((nCurrentU<<24)&0xff000000)|
                                    ((nCurrentY1<<16)&0x00ff0000)|
                                    ((nCurrentV<<8)&0x0000ff00)|
                                    ((nCurrentY2)&0x000000ff);

                                    *pOffsetOutput1=CurrentColour1;

                                    nCurrentU+=nIncU;
                                    nCurrentY1+=nIncY1;
                                    nCurrentV+=nIncV;
                                    nCurrentY2+=+nIncY2;

                                    pOffsetOutput1-=nWidth;
                                }

                                *ppCurrentLineHeight = pOffsetOutputStart1;


                                pCurrentSource+=1;
                                pCurrentOutput+=1;
                                ppCurrentLineHeight+=1;
                            }
                        }
                        else {
				while (pCurrentSource!=pSourceLineEnd) {
					const U32 SourceColour=*pCurrentSource;
					nSourceY1 = ((SourceColour&(0xff<<16))>>16)<<8;
					nSourceY1 -= 32640;								// 128*255
					nSourceY2 = ((SourceColour&(0xff<<0))>>0)<<8;
					nSourceY2 -= 32640;
				
					const int nOffset1=(nSourceY1*nOffsetScale)>>16;
					//const int nOffset2=(nSourceY2*nOffsetScale)>>16;
					
					U32* pOffsetOutputStart=pCurrentOutput+(nOffset1*nWidth);
					U32* pOffsetOutput=pOffsetOutputStart;
					U32* pPreviousOffsetOutput=*ppCurrentLineHeight;

					while ((pOffsetOutput<pOutputEnd)&&(pOffsetOutput>=pOutput)&&
										(pOffsetOutput>pPreviousOffsetOutput)) {
						*pOffsetOutput=SourceColour;
						pOffsetOutput-=nWidth;
					}

					*ppCurrentLineHeight=pOffsetOutputStart;

					pCurrentSource+=1;
					pCurrentOutput+=1;
					ppCurrentLineHeight+=1;
				}
			}
			pCurrentSource+=(nWidth*nLineGap);
			pCurrentOutput+=(nWidth*nLineGap);
		}

		U32* pFinalLineStart=pOutputEnd-nWidth;
		U32* pFinalLineEnd=pOutputEnd;
		U32** ppCurrentLineHeight=ppPreviousLineHeights;
		U32* pCurrentOutput=pFinalLineStart;

		while (pCurrentOutput<pFinalLineEnd) {
			U32* pPreviousOffsetOutput=*ppCurrentLineHeight;
			//
			const U32 SourceColour=*pPreviousOffsetOutput;
			U32* pOffsetOutput=pCurrentOutput;

			while ((pOffsetOutput<pOutputEnd)&&(pOffsetOutput>=pOutput)&&
								(pOffsetOutput>pPreviousOffsetOutput)) {
				*pOffsetOutput=SourceColour;
				pOffsetOutput-=nWidth;
			}
			/* does this really need to be done per yuv pixel?
			const U32 SourceColour=*pPreviousOffsetOutput;
			U32* pOffsetOutput1=pCurrentOutput;

			if ((pOffsetOutput1<pOutputEnd) && static_cast<int>(pOffsetOutput1>=pOutput)&&
								(pOffsetOutput1>pPreviousOffsetOutput)) {
						*pOffsetOutput1  = *pOffsetOutput1 & 0x000000ff;
						*pOffsetOutput1 |= SourceColour & 0xffffff00;
			}
				
			U32* pOffsetOutput2=(pCurrentOutput)+(nOffset2*nWidth);

			if ((pOffsetOutput2<pOutputEnd) && static_cast<int>(pOffsetOutput2>=pOutput)) {
						*pOffsetOutput2  = *pOffsetOutput2 & 0x00ff0000;
						*pOffsetOutput2 |= SourceColour & 0xff00ffff;
				}
			*/
			pCurrentOutput+=1;
			ppCurrentLineHeight+=1;
		}
	
    }
	image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_lumaoffset :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_lumaoffset::offsetCallback),
		  gensym("offset"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_lumaoffset::gapCallback),
		  gensym("gap"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_lumaoffset::fillCallback),
		  gensym("fill"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_lumaoffset::smoothCallback),
		  gensym("smooth"), A_DEFFLOAT, A_NULL);
}
void pix_lumaoffset :: offsetCallback(void *data, t_floatarg m_OffsetScale)
{
  GetMyClass(data)->m_OffsetScale=(m_OffsetScale);
  GetMyClass(data)->setPixModified();
}

void pix_lumaoffset :: gapCallback(void *data, t_floatarg m_LineGap)
{
  if(m_LineGap<0)m_LineGap=0.f;
  GetMyClass(data)->m_LineGap=(m_LineGap);
  GetMyClass(data)->setPixModified();
}
void pix_lumaoffset :: fillCallback(void *data, t_floatarg m_DoFilledLines)
{
  GetMyClass(data)->m_DoFilledLines=(m_DoFilledLines!=0.0);  
  GetMyClass(data)->setPixModified();
}

void pix_lumaoffset :: smoothCallback(void *data, t_floatarg m_DoSmoothFill)
{
  GetMyClass(data)->m_DoSmoothFill=(m_DoSmoothFill!=0.0);  
  GetMyClass(data)->setPixModified();
}
