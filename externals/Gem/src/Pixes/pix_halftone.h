/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Copyright (C) 2003-2004 ported by tigital@mac.com from "Pete's_Plugins"
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_HALFTONE_H_
#define _INCLUDE__GEM_PIXES_PIX_HALFTONE_H_

#include "Base/GemPixObj.h"
#include "Utils/GemMath.h"
	
enum {
    eRoundStyle,
    eLineStyle,
    eDiamondStyle,
    eEuclideanStyle,
    ePSDiamond
};

const int nMaxCellSize = 32;
unsigned char g_pDotFuncTable[nMaxCellSize*nMaxCellSize];
unsigned char g_pGreyScaleTable[512];
const int nFPShift=16;
const int nFPMult=(1<<nFPShift);

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_halftone
    
    View pix as halftone (like in newspaper-prints)

KEYWORDS
    pix
    
DESCRIPTION
   
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_halftone : public GemPixObj
{
    CPPEXTERN_HEADER(pix_halftone, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_halftone();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_halftone();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
		virtual void    processYUVImage(imageStruct &image);
  	    virtual void 	processGrayImage(imageStruct &image);

    	//////////
	//
	imageStruct    myImage;
	
	int 	m_CellSize;
	int 	m_Style;
	float 	m_Angle;
	int 	m_Smoothing;
	int	init;

	int nWidth;
	int nHeight;
	U32*				pSource;
	U32*				pOutput;
	
	struct SPete_HalfTone_Point {
	    int nX;
	    int nY;
	};

	struct SPete_HalfTone_Vertex {
	    SPete_HalfTone_Point Pos;
	    SPete_HalfTone_Point TexCoords;
	};

	int RoundDotFunc(float X,float Y, float scale);
	int LineDotFunc(float X,float Y, float scale);
	int DiamondDotFunc(float X,float Y, float scale);
	int EuclideanDotFunc(float X,float Y, float scale);
	int PSDiamondDotFunc(float X,float Y, float scale);
	int Init(int nWidth, int nHeight);
	void Pete_HalfTone_DeInit();
	void Rotate(SPete_HalfTone_Point* pinPoint,SPete_HalfTone_Point* poutPoint,float Angle);
	void Pete_HalfTone_MakeDotFuncTable(unsigned char* pDotFuncTableStart,int nCellSize,int nStyle, float scale);
	void Pete_HalfTone_CalcCorners(int nWidth,int nHeight,float AngleRadians,int nCellSize,
		SPete_HalfTone_Point* poutLeft,
		SPete_HalfTone_Point* poutRight,
		SPete_HalfTone_Point* poutTop,
		SPete_HalfTone_Point* poutBottom);
	void GetRasterizationPoints(
		SPete_HalfTone_Point* pinPoints,
		SPete_HalfTone_Point* poutLeft,
		SPete_HalfTone_Point* poutRight,
		SPete_HalfTone_Point* poutTop,
		SPete_HalfTone_Point* poutBottom);
	void HeightSortPoints(SPete_HalfTone_Point* pPoints,int nPointCount);
	void Pete_HalfTone_CalcSpanEnds(
		SPete_HalfTone_Point* pinLeft,
		SPete_HalfTone_Point* pinRight,
		SPete_HalfTone_Point* pinTop,
		SPete_HalfTone_Point* pinBottom,
		int nY,int* poutLeftX,int* poutRightX);
	void LerpAlongEdges(
		SPete_HalfTone_Point* pStart,
		SPete_HalfTone_Point* pMiddle,
		SPete_HalfTone_Point* pEnd,
		int nY,int* poutX);
	void RotateMultiple(SPete_HalfTone_Point* pinPoints,
		SPete_HalfTone_Point* poutPoints,
		int nPointCount,
		float Angle);
	void Pete_HalfTone_CalcSpanEnds_Vertex(
		SPete_HalfTone_Vertex* pinLeft,
		SPete_HalfTone_Vertex* pinRight,
		SPete_HalfTone_Vertex* pinTop,
		SPete_HalfTone_Vertex* pinBottom,
		int nY,
		SPete_HalfTone_Vertex* poutLeft,
		SPete_HalfTone_Vertex* poutRight);
	void Pete_HalfTone_LerpAlongEdges_Vertex(
		SPete_HalfTone_Vertex* pStart,
		SPete_HalfTone_Vertex* pMiddle,
		SPete_HalfTone_Vertex* pEnd,
		int nY,
		SPete_HalfTone_Vertex* poutVertex);
	void Pete_HalfTone_GetRasterizationVertices(
		SPete_HalfTone_Vertex* pinVertices,
		SPete_HalfTone_Vertex* poutLeft,
		SPete_HalfTone_Vertex* poutRight,
		SPete_HalfTone_Vertex* poutTop,
		SPete_HalfTone_Vertex* poutBottom);
	int Pete_HalfTone_GetLowestVertex(SPete_HalfTone_Vertex* pVertices,int nVertexCount);
	void Pete_HalfTone_RotateMultipleVertices(SPete_HalfTone_Vertex* pinVertices,
						  SPete_HalfTone_Vertex* poutVertices,
						  int nVertexCount,
						  float Angle);
	void Pete_HalfTone_MakeGreyScaleTable(unsigned char* pGreyScaleTableStart,int nSmoothingThreshold);
	void YUV_MakeGreyScaleTable(unsigned char* pGreyScaleTableStart,int nSmoothingThreshold);
	U32	Pete_GetImageAreaAverage(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,U32* pImageData,int nImageWidth,int nImageHeight);
	U32	GetImageAreaAverageLuma(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,U32* pImageData,int nImageWidth,int nImageHeight);
	unsigned char   Pete_GetImageAreaAverageGray(int nLeftX,int nTopY,int nDeltaX,int nDeltaY,unsigned char* pImageData,int nImageWidth,int nImageHeight);

    private:
    	//////////
    	// Static member functions
	static void 	sizeCallback(void *data, t_floatarg m_CellSize);
    	static void 	styleCallback(void *data, t_floatarg m_Style);
	static void 	smoothCallback(void *data, t_floatarg m_Smoothing);
	static void 	angleCallback(void *data, t_floatarg m_Angle); // 0..2pi 
	/* callbacks for normalized values: smooth=0..1; angle=0..360 */
	static void smoothNCallback(void *data, t_floatarg m_Smoothing);
	static void angleDEGCallback(void *data, t_floatarg m_Angle);
};

#endif	// for header file
