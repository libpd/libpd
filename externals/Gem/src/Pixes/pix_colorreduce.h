/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    Copyright (c) 2003 James Tittle
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_COLORREDUCE_H_
#define _INCLUDE__GEM_PIXES_PIX_COLORREDUCE_H_

#include "Base/GemPixObj.h"

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

extern "C" int Pete_ColorReduce_HistogramSortFunction(const void* pElem1,const void* pElem2);

/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_colorreduce
    
    

KEYWORDS
    pix
    
DESCRIPTION

    
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_colorreduce : public GemPixObj
{
    CPPEXTERN_HEADER(pix_colorreduce, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_colorreduce();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_colorreduce();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processGrayImage(imageStruct &image);
    	virtual void 	processYUVImage(imageStruct &image);


	imageStruct	myImage;
	int		nHeight;
	int		nWidth;
	int		init;

	U32*		pSource;
	U32*		pOutput;

	float	m_TargetColorCount;
	float	m_PalettePersistence;
	float	m_BoundarySmoothing;

	imageStruct tempImage;
	
	typedef void* 		SPete_MemHandle;
	SPete_MemHandle 	hRGBHistogram;
	SPete_MemHandle		hSortedColors;
	SPete_MemHandle		hInverseColorMap;
	
	int cnGridSizeShift;//=3;
	int cnGridSize;//=(1<<cnGridSizeShift);
	int cnGridSizeMask;//=(cnGridSize-1);
	int cnGridCellCount;//=(cnGridSize*cnGridSize*cnGridSize);
	int cnColorToIndexShift;//=(8-cnGridSizeShift);
	int cnGridCellWidth;//=(1<<cnColourToIndexShift);
	int cnGridCellHalfWidth;//=(cnGridCellWidth/2);
	int cnBiggestSignedInt;
	
	typedef struct _SPete_ColorReduce_InverseMapEntry {
	    U32	ClosestColor;
	    U32	NextClosestColor;
	} SPete_ColorReduce_InverseMapEntry;
	SPete_ColorReduce_InverseMapEntry*	pInvColorMapEntry;

	int Pete_ColorReduce_Init();
	void Pete_ColorReduce_DeInit();

	void Pete_ColorReduce_SortColors(int* pHistogram,int** ppSortedColors);
	void Pete_ColorReduce_SetupInverseColorMap(int** ppSortedColors,int nColors,SPete_ColorReduce_InverseMapEntry* pInverseColorMap,int* pHistogram);
	void Pete_ColorReduce_CalcHistogram(int nSampleSpacing,int* pHistogram,float PalettePersistence);
	inline U32 Pete_ColorReduce_GetClosestColor(U32 Color,SPete_ColorReduce_InverseMapEntry* pInverseColorMap,float BoundarySmoothing);

    private:
    
    	//////////
    	// Static member functions
    	static void 	countCallback(void *data, t_floatarg m_TargetColorCount);
	static void 	persistCallback(void *data, t_floatarg m_PalettePersistence);
	static void 	smoothCallback(void *data, t_floatarg m_BoundarySmoothing);
};

#endif	// for header file
