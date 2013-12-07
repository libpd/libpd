////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// tigital@mac.com
// ported from pete's_plugins
//
// Implementation file
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/PixPete.h"
#include "pix_backlight.h"

CPPEXTERN_NEW(pix_backlight);

/////////////////////////////////////////////////////////
//
// pix_backlight
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_backlight :: pix_backlight()
{
    m_SpikeScale = 127.0f;	// 0 to 255
    m_SpikeFloor = 0.0f;	// 0 to 255
    m_SpikeCeiling = 255.0f;	// 0 to 255
    init =0;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("scale"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("floor"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("ceiling"));

}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_backlight :: ~pix_backlight()
{
    //if(init) Pete_BackLight_DeInit();
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_backlight :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	//Pete_BackLight_Init();
	init = 1;
    }
    pSource = (U32*)image.data;
 
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
  
    pOutput = (U32*)myImage.data;

    const int nFixedShift=8;
//    const int nFixedMult=(1<<nFixedShift);

    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    const int nNumPixels = nWidth*nHeight;

    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
    const U32* pSourceEnd=(pSource+nNumPixels);
//    const U32* pOutputEnd=(pOutput+nNumPixels);

    Pete_ZeroMemory(pOutput,sizeof(U32)*nNumPixels);

    const int nSpikeScale=static_cast<int>(m_SpikeScale);
    const int nSpikeFloor=static_cast<int>(m_SpikeFloor);
    const int nSpikeCeiling=static_cast<int>(m_SpikeCeiling);

    int nY=0;
    while (pCurrentSource!=pSourceEnd) {
		
//	const U32* pSourceLineStart=pCurrentSource;
	const U32* pSourceLineEnd=pCurrentSource+nWidth;

	int nX=0;
	while (pCurrentSource!=pSourceLineEnd) {

	    U32 SourceColour=*pCurrentSource;
		
	    int nRed=(SourceColour&(0xff<<SHIFT_RED))>>SHIFT_RED;
	    int nGreen=(SourceColour&(0xff<<SHIFT_GREEN))>>SHIFT_GREEN;
	    int nBlue=(SourceColour&(0xff<<SHIFT_BLUE))>>SHIFT_BLUE;

	    int nLuminance = 
			((90 * nRed)+
			(115 * nGreen)+
			(51 * nBlue));

	    nLuminance>>=8;
//			SourceColour|=(nLuminance<<24);

	    nLuminance=clampFunc(nLuminance,nSpikeFloor,nSpikeCeiling);
	    nLuminance-=nSpikeFloor;

	    const int nLength=((nLuminance*nSpikeScale)>>nFixedShift);
	    
	    int nDeltaX=((nX-nHalfWidth)*nLength)>>8;
	    int nDeltaY=((nY-nHalfHeight)*nLength)>>8;

	    int nEndX=nX+nDeltaX;
	    if (nEndX>nWidth) {
		nEndX=nWidth;
	    } else if (nEndX<0) {
		nEndX=0;
	    }

	    int nEndY=nY+nDeltaY;
	    if (nEndY>nHeight) {
		nEndY=nHeight;
	    } else if (nEndY<0) {
		nEndY=0;
	    }

	    int nXInc;
	    if (nDeltaX<0) {
		nXInc=-1;
	    } else {
		nXInc=1;
	    }

	    int nYInc;
	    if (nDeltaY<0) {
		nYInc=-1;
	    } else {
		nYInc=1;
	    }

	    nDeltaX*=nXInc;
	    nDeltaY*=nYInc;

	    int nCurrentX=nX;
	    int nCurrentY=nY;

	    if ((nDeltaX==0)&&(nDeltaY==0)) {
		nDeltaX=1;
		nEndX+=1;
		nEndY+=1;
	    } else if (nDeltaX==0) {
		nEndX+=1;
	    } else if (nDeltaY==0) {
		nEndY+=1;
	    }

	    U32* pDest=(pOutput+(nCurrentY*nWidth)+nCurrentX);
	    const int nDestYInc=(nWidth*nYInc);
	    const int nDestXInc=nXInc;

	    if (nDeltaX>nDeltaY) {

		int	nCounter=nDeltaY;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {

#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			    (nCurrentX>=nWidth)||
			    (nCurrentY<0)||
			    (nCurrentY>=nHeight)||
			    (pDest<pOutput)||
			    (pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT
					
		    const U32 DestColour=*pDest;
		    if (DestColour<SourceColour) {
			*pDest=SourceColour;
		    } else {
			break;
		    }

		    if (nCounter>=nDeltaX) {
			nCounter-=nDeltaX;
			nCurrentY+=nYInc;
			pDest+=nDestYInc;
		    }
		    nCurrentX+=nXInc;
		    pDest+=nDestXInc;

		    nCounter+=nDeltaY;

		}


	    } else {

		int	nCounter=nDeltaX;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {


#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			(nCurrentX>=nWidth)||
			(nCurrentY<0)||
			(nCurrentY>=nHeight)||
			(pDest<pOutput)||
			(pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT

		    const U32 DestColour=*pDest;
		    if (DestColour<SourceColour) {
			*pDest=SourceColour;
		    } else {
			break;
		    }

		    if (nCounter>=nDeltaY) {
			nCounter-=nDeltaY;
			nCurrentX+=nXInc;
			pDest+=nDestXInc;
		    }
		    nCurrentY+=nYInc;
		    pDest+=nDestYInc;

		    nCounter+=nDeltaX;

		};
				
	    }

	    pCurrentSource+=1;
	    pCurrentOutput+=1;
	    nX+=1;

	}

	nY+=1;

    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// do the YUV processing here
//
/////////////////////////////////////////////////////////
void pix_backlight :: processYUVImage(imageStruct &image)
{
    nWidth = image.xsize/2;
    nHeight = image.ysize;
    if (!init) {
	//Pete_BackLight_Init();
	init = 1;
    }
    pSource = (U32*)image.data;
    
    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = (U32*)myImage.data;

    const int nFixedShift=8;
//    const int nFixedMult=(1<<nFixedShift);

    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    const int nNumPixels = nWidth*nHeight;

    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
    const U32* pSourceEnd=(pSource+nNumPixels);
//    const U32* pOutputEnd=(pOutput+nNumPixels);

//Pete_ZeroMemory(pOutput,sizeof(U32)*nNumPixels);

    unsigned char *zero;
    
    zero = (unsigned char*)pOutput;
    int temp = nNumPixels;
    
    while(--temp>>1){
        *zero++ = 128; *zero++ =0;
        *zero++ = 128; *zero++ = 0;
    }

    union {
        unsigned char c[4];
        unsigned int  i;
    }bu;

    bu.c[0] = 128;
    bu.c[1] = 0;
    bu.c[2] = 128;
    bu.c[3] = 0;

    unsigned int black;

    black = bu.i;
    
    const int nSpikeScale=static_cast<int>(m_SpikeScale);
    const int nSpikeFloor=static_cast<int>(m_SpikeFloor);
    const int nSpikeCeiling=static_cast<int>(m_SpikeCeiling);

    int nY=0;
    while (pCurrentSource!=pSourceEnd) {
		
//	const U32* pSourceLineStart=pCurrentSource;
	const U32* pSourceLineEnd=pCurrentSource+nWidth;

	int nX=0;
	while (pCurrentSource!=pSourceLineEnd) {

	    U32 SourceColour=*pCurrentSource;
		
	    int nLuminance=(SourceColour&(0xff<<SHIFT_Y1))>>SHIFT_Y1;
            
	    nLuminance=clampFunc(nLuminance,nSpikeFloor,nSpikeCeiling);
	    nLuminance-=nSpikeFloor;

	    const int nLength=((nLuminance*nSpikeScale)>>nFixedShift);
	    
	    int nDeltaX=((nX-nHalfWidth)*nLength)>>8;
	    int nDeltaY=((nY-nHalfHeight)*nLength)>>8;

	    int nEndX=nX+nDeltaX;
	    if (nEndX>nWidth) {
		nEndX=nWidth;
	    } else if (nEndX<0) {
		nEndX=0;
	    }

	    int nEndY=nY+nDeltaY;
	    if (nEndY>nHeight) {
		nEndY=nHeight;
	    } else if (nEndY<0) {
		nEndY=0;
	    }

	    int nXInc;
	    if (nDeltaX<0) {
		nXInc=-1;
	    } else {
		nXInc=1;
	    }

	    int nYInc;
	    if (nDeltaY<0) {
		nYInc=-1;
	    } else {
		nYInc=1;
	    }

	    nDeltaX*=nXInc;
	    nDeltaY*=nYInc;

	    int nCurrentX=nX;
	    int nCurrentY=nY;

	    if ((nDeltaX==0)&&(nDeltaY==0)) {
		nDeltaX=1;
		nEndX+=1;
		nEndY+=1;
	    } else if (nDeltaX==0) {
		nEndX+=1;
	    } else if (nDeltaY==0) {
		nEndY+=1;
	    }

	    U32* pDest=(pOutput+(nCurrentY*nWidth)+nCurrentX);
	    const int nDestYInc=(nWidth*nYInc);
	    const int nDestXInc=nXInc;

	    if (nDeltaX>nDeltaY) {

		int	nCounter=nDeltaY;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {

#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			    (nCurrentX>=nWidth)||
			    (nCurrentY<0)||
			    (nCurrentY>=nHeight)||
			    (pDest<pOutput)||
			    (pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT
					
		    const U32 DestColour=*pDest;

                    
		    if (DestColour<SourceColour || DestColour == black) {
			*pDest=SourceColour;
		    } else {
			break;
		    }

		    if (nCounter>=nDeltaX) {
			nCounter-=nDeltaX;
			nCurrentY+=nYInc;
			pDest+=nDestYInc;
		    }
		    nCurrentX+=nXInc;
		    pDest+=nDestXInc;

		    nCounter+=nDeltaY;

		}


	    } else {

		int	nCounter=nDeltaX;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {


#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			(nCurrentX>=nWidth)||
			(nCurrentY<0)||
			(nCurrentY>=nHeight)||
			(pDest<pOutput)||
			(pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT

		    const U32 DestColour=*pDest;
                    
                   //  if (DestColour<SourceColour) {
                    if (DestColour<SourceColour || DestColour == black) {
                         *pDest=SourceColour;
                     } else {
                         break;
                     }
		    if (nCounter>=nDeltaY) {
			nCounter-=nDeltaY;
			nCurrentX+=nXInc;
			pDest+=nDestXInc;
		    }
		    nCurrentY+=nYInc;
		    pDest+=nDestYInc;

		    nCounter+=nDeltaX;

		};
				
	    }

	    pCurrentSource+=1;
	    pCurrentOutput+=1;
	    nX+=1;

	}

	nY+=1;

    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// do the Gray processing here
//
/////////////////////////////////////////////////////////
/*
void pix_backlight :: processGrayImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	//Pete_BackLight_Init();
	init = 1;
    }
    unsigned char* pSource = image.data;
    
    if ( myImage.xsize*myImage.ysize*myImage.csize != image.xsize*image.ysize*image.csize ){
	int dataSize = image.xsize * image.ysize * image.csize;
	myImage.clear();

	myImage.allocate(dataSize);
    }

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.csize = image.csize;
    myImage.type  = image.type;
    unsigned char* pOutput = myImage.data;

    const int nFixedShift=8;
//    const int nFixedMult=(1<<nFixedShift);

    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    const int nNumPixels = nWidth*nHeight;

    U32* pCurrentSource=pSource;
    U32* pCurrentOutput=pOutput;
    const U32* pSourceEnd=(pSource+nNumPixels);
//    const U32* pOutputEnd=(pOutput+nNumPixels);

    Pete_ZeroMemory(pOutput,sizeof(U32)*nNumPixels);

    const int nSpikeScale=static_cast<int>(m_SpikeScale);
    const int nSpikeFloor=static_cast<int>(m_SpikeFloor);
    const int nSpikeCeiling=static_cast<int>(m_SpikeCeiling);

    int nY=0;
    while (pCurrentSource!=pSourceEnd) {
		
//	const U32* pSourceLineStart=pCurrentSource;
	const U32* pSourceLineEnd=pCurrentSource+nWidth;

	int nX=0;
	while (pCurrentSource!=pSourceLineEnd) {

	    U32 SourceColour=*pCurrentSource;
		
	    int nRed=(SourceColour&(0xff<<SHIFT_RED))>>SHIFT_RED;
	    int nGreen=(SourceColour&(0xff<<SHIFT_GREEN))>>SHIFT_GREEN;
	    int nBlue=(SourceColour&(0xff<<SHIFT_BLUE))>>SHIFT_BLUE;

	    int nLuminance = 
			((90 * nRed)+
			(115 * nGreen)+
			(51 * nBlue));

	    nLuminance>>=8;
//			SourceColour|=(nLuminance<<24);

	    nLuminance=clampFunc(nLuminance,nSpikeFloor,nSpikeCeiling);
	    nLuminance-=nSpikeFloor;

	    const int nLength=((nLuminance*nSpikeScale)>>nFixedShift);
	    
	    int nDeltaX=((nX-nHalfWidth)*nLength)>>8;
	    int nDeltaY=((nY-nHalfHeight)*nLength)>>8;

	    int nEndX=nX+nDeltaX;
	    if (nEndX>nWidth) {
		nEndX=nWidth;
	    } else if (nEndX<0) {
		nEndX=0;
	    }

	    int nEndY=nY+nDeltaY;
	    if (nEndY>nHeight) {
		nEndY=nHeight;
	    } else if (nEndY<0) {
		nEndY=0;
	    }

	    int nXInc;
	    if (nDeltaX<0) {
		nXInc=-1;
	    } else {
		nXInc=1;
	    }

	    int nYInc;
	    if (nDeltaY<0) {
		nYInc=-1;
	    } else {
		nYInc=1;
	    }

	    nDeltaX*=nXInc;
	    nDeltaY*=nYInc;

	    int nCurrentX=nX;
	    int nCurrentY=nY;

	    if ((nDeltaX==0)&&(nDeltaY==0)) {
		nDeltaX=1;
		nEndX+=1;
		nEndY+=1;
	    } else if (nDeltaX==0) {
		nEndX+=1;
	    } else if (nDeltaY==0) {
		nEndY+=1;
	    }

	    U32* pDest=(pOutput+(nCurrentY*nWidth)+nCurrentX);
	    const int nDestYInc=(nWidth*nYInc);
	    const int nDestXInc=nXInc;

	    if (nDeltaX>nDeltaY) {

		int	nCounter=nDeltaY;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {

#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			    (nCurrentX>=nWidth)||
			    (nCurrentY<0)||
			    (nCurrentY>=nHeight)||
			    (pDest<pOutput)||
			    (pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT
					
		    const U32 DestColour=*pDest;
		    if (DestColour<SourceColour) {
			*pDest=SourceColour;
		    } else {
			break;
		    }

		    if (nCounter>=nDeltaX) {
			nCounter-=nDeltaX;
			nCurrentY+=nYInc;
			pDest+=nDestYInc;
		    }
		    nCurrentX+=nXInc;
		    pDest+=nDestXInc;

		    nCounter+=nDeltaY;

		}


	    } else {

		int	nCounter=nDeltaX;

		while ((nCurrentX!=nEndX)&&(nCurrentY!=nEndY)) {


#ifdef PETE_DEBUG_BACKLIGHT
		    if ((nCurrentX<0)||
			(nCurrentX>=nWidth)||
			(nCurrentY<0)||
			(nCurrentY>=nHeight)||
			(pDest<pOutput)||
			(pDest>=pOutputEnd)) {

			while (true); // Pete- Infinite loop, easy way to tell if this triggered!

		    }
#endif // PETE_DEBUG_BACKLIGHT

		    const U32 DestColour=*pDest;
		    if (DestColour<SourceColour) {
			*pDest=SourceColour;
		    } else {
			break;
		    }

		    if (nCounter>=nDeltaY) {
			nCounter-=nDeltaY;
			nCurrentX+=nXInc;
			pDest+=nDestXInc;
		    }
		    nCurrentY+=nYInc;
		    pDest+=nDestYInc;

		    nCounter+=nDeltaX;

		};
				
	    }

	    pCurrentSource+=1;
	    pCurrentOutput+=1;
	    nX+=1;

	}

	nY+=1;

    }

    image.data = myImage.data;
}
*/

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_backlight :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_backlight::scaleCallback),
		  gensym("scale"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_backlight::floorCallback),
		  gensym("floor"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_backlight::ceilingCallback),
		  gensym("ceiling"), A_DEFFLOAT, A_NULL);
}
void pix_backlight :: scaleCallback(void *data, t_floatarg m_SpikeScale)
{
  m_SpikeScale*=255.0;
  //  if(m_SpikeScale<0.f)m_SpikeScale=0.f;else if(m_SpikeScale>255.f)m_SpikeScale=255.f;
  GetMyClass(data)->m_SpikeScale=(m_SpikeScale);
  GetMyClass(data)->setPixModified();
}

void pix_backlight :: floorCallback(void *data, t_floatarg m_SpikeFloor)
{
  m_SpikeFloor*=255.0;
  if(m_SpikeFloor<0.f)m_SpikeFloor=0.f;else if(m_SpikeFloor>255.f)m_SpikeFloor=255.f;
  GetMyClass(data)->m_SpikeFloor=(m_SpikeFloor);
  GetMyClass(data)->setPixModified();
}

void pix_backlight :: ceilingCallback(void *data, t_floatarg m_SpikeCeiling)
{
  m_SpikeCeiling*=255.0;
  if(m_SpikeCeiling<0.f)m_SpikeCeiling=0.f;else if(m_SpikeCeiling>255.f)m_SpikeCeiling=255.f;
  GetMyClass(data)->m_SpikeCeiling=(m_SpikeCeiling);  
  GetMyClass(data)->setPixModified();
}
