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
#include "pix_refraction.h"
	
CPPEXTERN_NEW(pix_refraction);

/////////////////////////////////////////////////////////
//
// pix_refraction
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_refraction :: pix_refraction()
{ 
    m_Refraction = 2.0f;
    m_CellWidth = 16.0f;
    m_CellHeight = 16.0f;
    m_DoAllowMagnification = 1.0f;

    init =0;
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_refraction :: ~pix_refraction()
{ 
    
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_refraction :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	init = 1;
    }
    pSource = (U32*)image.data;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
  
    pOutput = (U32*)myImage.data;
	
    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    const int nNumPixels = nWidth*nHeight;

    float GatedRefraction=m_Refraction;
    if ((m_DoAllowMagnification==0.0f)&&(m_Refraction<1.0f)) {
		GatedRefraction=1.0f;
    }

    const int nRefraction=static_cast<int>(GatedRefraction*256.0f);

    int nCellWidth=static_cast<int>(m_CellWidth);
    if (nCellWidth<=0) {
		nCellWidth=1;
    }
    int nCellHeight=static_cast<int>(m_CellHeight);
    if (nCellHeight<=0) {
		nCellHeight=1;
    }

    const int nHalfCellWidth=(nCellWidth/2);
    const int nHalfCellHeight=(nCellHeight/2);	

    //const int nYStartOffset=(nHalfCellHeight*(256))/nRefraction;
    //const int nXStartOffset=(nHalfCellWidth*(256))/nRefraction;

    U32* pCurrentOutput=pOutput;
    const U32* pOutputEnd=(pOutput+nNumPixels);

    int nY=-nHalfHeight+nHalfCellHeight;
    while (pCurrentOutput!=pOutputEnd) {

	//const U32* pOutputLineStart=pCurrentOutput;
	const U32* pOutputLineEnd=pCurrentOutput+nWidth;

	const int nYCentre=(((nY+(GetSign(nY)*nHalfCellHeight))/nCellHeight)*nCellHeight)+nHalfCellHeight;
	const int nYDist=(nY-nYCentre);
	int nSourceY=((nYDist*nRefraction)>>8)+nYCentre+nHalfHeight;
	nSourceY=clampFunc(nSourceY,0,(nHeight-1));

	const U32* pSourceLineStart=(pSource+(nSourceY*nWidth));

	int nX=-nHalfWidth+nHalfCellWidth;
	while (pCurrentOutput!=pOutputLineEnd) {

	    const int nXCentre=(((nX+(GetSign(nX)*nHalfCellWidth))/nCellWidth)*nCellWidth)+nHalfCellWidth;
	    const int nXDist=(nX-nXCentre);
	    int nSourceX=((nXDist*nRefraction)>>8)+nXCentre+nHalfWidth;
	    nSourceX=clampFunc(nSourceX,0,(nWidth-1));

	    const U32* pCurrentSource=pSourceLineStart+nSourceX;

	    *pCurrentOutput=*pCurrentSource;

	    pCurrentOutput+=1;
	    nX+=1;

	}

	nY+=1;

    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// processYUVImage
//
/////////////////////////////////////////////////////////
void pix_refraction :: processYUVImage(imageStruct &image)
{
    nWidth = image.xsize/2;
    nHeight = image.ysize;
    if (!init) {
	init = 1;
    }
    pSource = (U32*)image.data;

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.setCsizeByFormat(image.format);
    myImage.reallocate();
    pOutput = (U32*)myImage.data;
	
    const int nHalfWidth=(nWidth/2);
    const int nHalfHeight=(nHeight/2);

    const int nNumPixels = nWidth*nHeight;

    float GatedRefraction=m_Refraction;
    if ((m_DoAllowMagnification==0.0f)&&(m_Refraction<1.0f)) {
		GatedRefraction=1.0f;
    }

    const int nRefraction=static_cast<int>(GatedRefraction*256.0f);

    int nCellWidth=static_cast<int>(m_CellWidth);
    if (nCellWidth<=0) {
		nCellWidth=1;
    }
    int nCellHeight=static_cast<int>(m_CellHeight);
    if (nCellHeight<=0) {
		nCellHeight=1;
    }

    const int nHalfCellWidth=(nCellWidth/2);
    const int nHalfCellHeight=(nCellHeight/2);	

    //const int nYStartOffset=(nHalfCellHeight*(256))/nRefraction;
    //const int nXStartOffset=(nHalfCellWidth*(256))/nRefraction;

    U32* pCurrentOutput=pOutput;
    const U32* pOutputEnd=(pOutput+nNumPixels);

    int nY=-nHalfHeight+nHalfCellHeight;
    while (pCurrentOutput!=pOutputEnd) {

	//const U32* pOutputLineStart=pCurrentOutput;
	const U32* pOutputLineEnd=pCurrentOutput+nWidth;

	const int nYCentre=(((nY+(GetSign(nY)*nHalfCellHeight))/nCellHeight)*nCellHeight)+nHalfCellHeight;
	const int nYDist=(nY-nYCentre);
	int nSourceY=((nYDist*nRefraction)>>8)+nYCentre+nHalfHeight;
	nSourceY=clampFunc(nSourceY,0,(nHeight-1));

	const U32* pSourceLineStart=(pSource+(nSourceY*nWidth));

	int nX=-nHalfWidth+nHalfCellWidth;
	while (pCurrentOutput!=pOutputLineEnd) {

	    const int nXCentre=(((nX+(GetSign(nX)*nHalfCellWidth))/nCellWidth)*nCellWidth)+nHalfCellWidth;
	    const int nXDist=(nX-nXCentre);
	    int nSourceX=((nXDist*nRefraction)>>8)+nXCentre+nHalfWidth;
	    nSourceX=clampFunc(nSourceX,0,(nWidth-1));

	    const U32* pCurrentSource=pSourceLineStart+nSourceX;

	    *pCurrentOutput=*pCurrentSource;

	    pCurrentOutput+=1;
	    nX+=1;

	}

	nY+=1;

    }
    image.data = myImage.data;
}

/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_refraction :: obj_setupCallback(t_class *classPtr)
{
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_refraction::refractCallback),
		  gensym("refract"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_refraction::widthCallback),
		  gensym("width"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_refraction::heightCallback),
		  gensym("height"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_refraction::magCallback),
		  gensym("mag"), A_DEFFLOAT, A_NULL);
}
void pix_refraction :: refractCallback(void *data, t_floatarg m_Refraction)
{
  GetMyClass(data)->m_Refraction=(m_Refraction);
  GetMyClass(data)->setPixModified();
}

void pix_refraction :: widthCallback(void *data, t_floatarg m_CellWidth)
{
  GetMyClass(data)->m_CellWidth=(m_CellWidth);
  GetMyClass(data)->setPixModified();
}
void pix_refraction :: heightCallback(void *data, t_floatarg m_CellHeight)
{
  GetMyClass(data)->m_CellHeight=(m_CellHeight);  
  GetMyClass(data)->setPixModified();
}

void pix_refraction :: magCallback(void *data, t_floatarg m_DoAllowMagnification)
{
  GetMyClass(data)->m_DoAllowMagnification=(m_DoAllowMagnification);  
  GetMyClass(data)->setPixModified();
}
