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
#include "pix_colorreduce.h"

CPPEXTERN_NEW(pix_colorreduce);

/////////////////////////////////////////////////////////
//
// pix_colorreduce
//
/////////////////////////////////////////////////////////
// Constructor
//
/////////////////////////////////////////////////////////
pix_colorreduce :: pix_colorreduce() : 
  hRGBHistogram(NULL), hSortedColors(NULL), hInverseColorMap(NULL)
{ 
    m_TargetColorCount = 8.0f; 		// 1 to 255
    m_PalettePersistence = 0.95f;	// 0 to 1
    m_BoundarySmoothing = 0.10f;	// 0 or 1
    
    cnGridSizeShift=3;
    cnGridSize=(1<<cnGridSizeShift);
    cnGridSizeMask=(cnGridSize-1);
    cnGridCellCount=(cnGridSize*cnGridSize*cnGridSize);
    cnColorToIndexShift=(8-cnGridSizeShift);
    cnGridCellWidth=(1<<cnColorToIndexShift);
    cnGridCellHalfWidth=(cnGridCellWidth/2);
    cnBiggestSignedInt=0x7fffffff;

    init =0;

    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("count"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("persist"));
    inlet_new(this->x_obj, &this->x_obj->ob_pd, gensym("float"), gensym("smooth"));

    tempImage.xsize=0;
    tempImage.ysize=0;
    tempImage.setCsizeByFormat(GL_RGBA);
}

/////////////////////////////////////////////////////////
// Destructor
//
/////////////////////////////////////////////////////////
pix_colorreduce :: ~pix_colorreduce()
{
    if(init) Pete_ColorReduce_DeInit();
}

/////////////////////////////////////////////////////////
// processImage
//
/////////////////////////////////////////////////////////
void pix_colorreduce :: processYUVImage(imageStruct &image){

  tempImage.xsize=image.xsize;
  tempImage.ysize=image.ysize;
#ifndef __APPLE__
  tempImage.format=GL_RGBA;
#else
  tempImage.format=GL_BGRA_EXT;
#endif
  tempImage.fromUYVY(image.data);

  processRGBAImage(tempImage);

  image.fromRGBA(tempImage.data);
}

void pix_colorreduce :: processGrayImage(imageStruct &image){
  tempImage.xsize=image.xsize;
  tempImage.ysize=image.ysize;
  tempImage.fromGray(image.data);

  processRGBAImage(tempImage);

  image.fromRGBA(tempImage.data);
}




void pix_colorreduce :: processRGBAImage(imageStruct &image)
{
    nWidth = image.xsize;
    nHeight = image.ysize;
    if (!init) {
	Pete_ColorReduce_Init();
	init = 1;
    }
    pSource = reinterpret_cast<U32*>(image.data);

    myImage.xsize = image.xsize;
    myImage.ysize = image.ysize;
    myImage.csize = image.csize;
    myImage.type  = image.type;
    myImage.reallocate();
    pOutput = reinterpret_cast<U32*>(myImage.data);
	
    const int nColors=static_cast<int>(m_TargetColorCount);
    const float PalettePersistence=m_PalettePersistence;
    const float BoundarySmoothing=m_BoundarySmoothing;

    int* pHistogram=(int*)Pete_LockHandle(hRGBHistogram);
    if (pHistogram==NULL) {
		return;
    }
    int** ppSortedColors=(int**)Pete_LockHandle(hSortedColors);
    if (ppSortedColors==NULL) {
		return;
    }
    SPete_ColorReduce_InverseMapEntry* pInverseColorMap=
		(SPete_ColorReduce_InverseMapEntry*)Pete_LockHandle(hInverseColorMap);
    if (pInverseColorMap==NULL) {
		return;
    }
	
    const int nSampleSpacing=4;

    Pete_ColorReduce_CalcHistogram(nSampleSpacing,pHistogram,PalettePersistence);

    Pete_ColorReduce_SortColors(pHistogram,ppSortedColors);

    Pete_ColorReduce_SetupInverseColorMap(ppSortedColors,nColors,pInverseColorMap,pHistogram);

    const int nNumPixels=(nWidth*nHeight);

    U32* pCurrentSource=pSource;
    U32* pSourceEnd=pSource+nNumPixels;

    U32* pCurrentOutput=pOutput;

    while (pCurrentSource<pSourceEnd) {

	*pCurrentOutput=Pete_ColorReduce_GetClosestColor(*pCurrentSource,pInverseColorMap,BoundarySmoothing);

	pCurrentSource+=1;
	pCurrentOutput+=1;

    }
	
    Pete_UnLockHandle(hRGBHistogram);
    Pete_UnLockHandle(hSortedColors);
    Pete_UnLockHandle(hInverseColorMap);

    image.data = myImage.data;
}
/////////////////////////////////////////////////////////
// do other processing here
//
/////////////////////////////////////////////////////////
int pix_colorreduce :: Pete_ColorReduce_Init() {

	Pete_ColorReduce_DeInit();

	hRGBHistogram=Pete_NewHandle(cnGridCellCount*sizeof(int));
	if (hRGBHistogram==NULL) {
		Pete_ColorReduce_DeInit();
		return 0;
	}
	Pete_ZeroMemory((char*)Pete_LockHandle(hRGBHistogram),cnGridCellCount*sizeof(int));

	hSortedColors=Pete_NewHandle(cnGridCellCount*sizeof(int*));
	if (hSortedColors==NULL) {
		Pete_ColorReduce_DeInit();
		return 0;
	}

	hInverseColorMap=
		Pete_NewHandle(cnGridCellCount*sizeof(SPete_ColorReduce_InverseMapEntry));

	if (hInverseColorMap==NULL) {
		Pete_ColorReduce_DeInit();
		return 0;
	}

	return 1;
}

void pix_colorreduce :: Pete_ColorReduce_DeInit() {
	if (hRGBHistogram!=NULL) {
		Pete_FreeHandle(hRGBHistogram);
		hRGBHistogram=NULL;
	}

	if (hSortedColors!=NULL) {
		Pete_FreeHandle(hSortedColors);
		hSortedColors=NULL;
	}

	if (hInverseColorMap!=NULL) {
		Pete_FreeHandle(hInverseColorMap);
		hInverseColorMap=NULL;
	}

}

inline U32 pix_colorreduce :: Pete_ColorReduce_GetClosestColor(U32 Color,SPete_ColorReduce_InverseMapEntry* pInverseColorMap,float BoundarySmoothing) {

	const int nRed=(Color>>SHIFT_RED)&0xff;
	const int nGreen=(Color>>SHIFT_GREEN)&0xff;
	const int nBlue=(Color>>SHIFT_BLUE)&0xff;

	const int nRedIndex=(nRed>>cnColorToIndexShift);
	const int nGreenIndex=(nGreen>>cnColorToIndexShift);
	const int nBlueIndex=(nBlue>>cnColorToIndexShift);

	SPete_ColorReduce_InverseMapEntry* pInvColorMapEntry=
		pInverseColorMap+
		(nBlueIndex<<(cnGridSizeShift*2))+
		(nGreenIndex<<(cnGridSizeShift*1))+
		(nRedIndex<<(cnGridSizeShift*0));

	U32 ClosestColor=(pInvColorMapEntry->ClosestColor);

	if (BoundarySmoothing==0.0f) {
		return ClosestColor;
	}
	
	U32 NextClosestColor=(pInvColorMapEntry->NextClosestColor);

	const int nClosestRed=(ClosestColor>>SHIFT_RED)&0xff;
	const int nClosestGreen=(ClosestColor>>SHIFT_GREEN)&0xff;
	const int nClosestBlue=(ClosestColor>>SHIFT_BLUE)&0xff;

	const int nNextClosestRed=(NextClosestColor>>SHIFT_RED)&0xff;
	const int nNextClosestGreen=(NextClosestColor>>SHIFT_GREEN)&0xff;
	const int nNextClosestBlue=(NextClosestColor>>SHIFT_BLUE)&0xff;

	const int nClosestDeltaRed=(nClosestRed-nRed);
	const int nClosestDeltaGreen=(nClosestGreen-nGreen);
	const int nClosestDeltaBlue=(nClosestBlue-nBlue);

	const int nNextClosestDeltaRed=(nNextClosestRed-nRed);
	const int nNextClosestDeltaGreen=(nNextClosestGreen-nGreen);
	const int nNextClosestDeltaBlue=(nNextClosestBlue-nBlue);

	const int nClosestDistSqrd=
		(nClosestDeltaRed*nClosestDeltaRed)+
		(nClosestDeltaGreen*nClosestDeltaGreen)+
		(nClosestDeltaBlue*nClosestDeltaBlue);

	const int nNextClosestDistSqrd=
		(nNextClosestDeltaRed*nNextClosestDeltaRed)+
		(nNextClosestDeltaGreen*nNextClosestDeltaGreen)+
		(nNextClosestDeltaBlue*nNextClosestDeltaBlue);

	const float ClosestDist=(float)sqrt((double)nClosestDistSqrd);
	const float NextClosestDist=(float)sqrt((double)nNextClosestDistSqrd);
	const float TotalDist=(ClosestDist+NextClosestDist);

	if (TotalDist==0.0f) {
		return ClosestColor;
	}

	const float UnWeightedLerpValue=(NextClosestDist/TotalDist);
	
	float WeightedLerpValue=(UnWeightedLerpValue-0.5f)/BoundarySmoothing;
	WeightedLerpValue+=0.5f;
	if (WeightedLerpValue>1.0f) {
		return ClosestColor;
	} else if (WeightedLerpValue<0.0f) {
		return NextClosestColor;
	} else {

		const float OneMinusLerpValue=(1.0f-WeightedLerpValue);

		const int nSmoothedRed=static_cast<int>(
			(nClosestRed*WeightedLerpValue)+
			(nNextClosestRed*OneMinusLerpValue));

		const int nSmoothedGreen=static_cast<int>(
			(nClosestGreen*WeightedLerpValue)+
			(nNextClosestGreen*OneMinusLerpValue));

		const int nSmoothedBlue=static_cast<int>(
			(nClosestBlue*WeightedLerpValue)+
			(nNextClosestBlue*OneMinusLerpValue));

		const U32 SmoothedColor=
			(nSmoothedRed<<SHIFT_RED)|
			(nSmoothedGreen<<SHIFT_GREEN)|
			(nSmoothedBlue<<SHIFT_BLUE);

		return SmoothedColor;

	}
}
void pix_colorreduce :: Pete_ColorReduce_CalcHistogram(
	int nSampleSpacing,
	int* pHistogram,
	float PalettePersistence) {

	const int nHistogramMult=
		static_cast<int>((1<<8)*PalettePersistence);

	int nCount;
	for (nCount=0; nCount<cnGridCellCount; nCount+=1) {

		int nEntry=pHistogram[nCount];

		nEntry*=nHistogramMult;
		nEntry>>=8;

		pHistogram[nCount]=nEntry;

	}

	const int nNumPixels = nWidth*nHeight;
	
	U32* pCurrentSource=pSource;
	const U32* pSourceEnd=(pSource+nNumPixels);

	while (pCurrentSource<pSourceEnd) {
		
		U32* pSourceLineStart=pCurrentSource;
		const U32* pSourceLineEnd=pCurrentSource+nWidth;
			
		while (pCurrentSource<pSourceLineEnd) {

			U32 SourceColor=*pCurrentSource;
			
			const int nSourceRed=(SourceColor>>SHIFT_RED)&0xff;
			const int nSourceGreen=(SourceColor>>SHIFT_GREEN)&0xff;
			const int nSourceBlue=(SourceColor>>SHIFT_BLUE)&0xff;

			const int nRedIndex=(nSourceRed>>cnColorToIndexShift);
			const int nGreenIndex=(nSourceGreen>>cnColorToIndexShift);
			const int nBlueIndex=(nSourceBlue>>cnColorToIndexShift);

			int* pResultCell=
				(pHistogram+
				(nBlueIndex*cnGridSize*cnGridSize)+
				(nGreenIndex*cnGridSize)+
				(nRedIndex));

			(*pResultCell)+=1;

			pCurrentSource+=nSampleSpacing;

		}

		pCurrentSource=
			pSourceLineStart+(nSampleSpacing*nWidth);

	}

}

extern "C" int Pete_ColorReduce_HistogramSortFunction(const void* pElem1,const void* pElem2) {

	int** ppFirstElement=(int**)pElem1;
	int** ppSecondElement=(int**)pElem2;

	const int nFirstValue=(**ppFirstElement);
	const int nSecondValue=(**ppSecondElement);

	if (nSecondValue<nFirstValue) {
		return -1;
	} else if (nSecondValue>nFirstValue) {
		return 1;
	} else {
		return 0;
	}

}

void pix_colorreduce :: Pete_ColorReduce_SortColors(int* pHistogram,int** ppSortedColors) {

	int nCount;
	for (nCount=0; nCount<cnGridCellCount; nCount+=1) {

		ppSortedColors[nCount]=&(pHistogram[nCount]);
	
	}

	qsort((void*)ppSortedColors,cnGridCellCount,sizeof(int*),&Pete_ColorReduce_HistogramSortFunction);

}

void pix_colorreduce :: Pete_ColorReduce_SetupInverseColorMap(int** ppSortedColors,int nColors,SPete_ColorReduce_InverseMapEntry* pInverseColorMap,int* pHistogram) {

	int *nSortedRed  = new int[cnGridCellCount];
	int *nSortedGreen= new int[cnGridCellCount];
	int *nSortedBlue = new int[cnGridCellCount];

	int nCount;
	for (nCount=0; nCount<nColors; nCount+=1) {

		int nOffset=(ppSortedColors[nCount]-pHistogram);

		nSortedRed[nCount]=(nOffset>>(cnGridSizeShift*0))&cnGridSizeMask;
		nSortedRed[nCount]*=cnGridCellWidth;

		nSortedGreen[nCount]=(nOffset>>(cnGridSizeShift*1))&cnGridSizeMask;
		nSortedGreen[nCount]*=cnGridCellWidth;

		nSortedBlue[nCount]=(nOffset>>(cnGridSizeShift*2))&cnGridSizeMask;
		nSortedBlue[nCount]*=cnGridCellWidth;

	}

	int nBlueIndex;
	for (nBlueIndex=0; nBlueIndex<cnGridSize; nBlueIndex+=1) {

		int nBlue=(nBlueIndex*cnGridCellWidth)+(cnGridCellHalfWidth);

		int nGreenIndex;
		for (nGreenIndex=0; nGreenIndex<cnGridSize; nGreenIndex+=1) {

			int nGreen=(nGreenIndex*cnGridCellWidth)+(cnGridCellHalfWidth);

			int nRedIndex;
			for (nRedIndex=0; nRedIndex<cnGridSize; nRedIndex+=1) {

				int nRed=(nRedIndex*cnGridCellWidth)+(cnGridCellHalfWidth);
			
				int nClosestDistance=cnBiggestSignedInt;
				int nNextClosestDistance=cnBiggestSignedInt;
				U32 ResultColor = 0;
				U32 NextClosestColor = 0;

				int nCount;
				for (nCount=0; nCount<nColors; nCount+=1) {

					const int nCandRed=nSortedRed[nCount];
					const int nCandGreen=nSortedGreen[nCount];
					const int nCandBlue=nSortedBlue[nCount];

					const int nRedDist=nRed-nCandRed;
					const int nRedDistSquared=(nRedDist*nRedDist);
					
					const int nGreenDist=nGreen-nCandGreen;
					const int nGreenDistSquared=(nGreenDist*nGreenDist);

					const int nBlueDist=nBlue-nCandBlue;
					const int nBlueDistSquared=(nBlueDist*nBlueDist);

					const int nTotalDistSquared=
						nRedDistSquared+
						nGreenDistSquared+
						nBlueDistSquared;

					if (nTotalDistSquared<nClosestDistance) {

						nNextClosestDistance=nClosestDistance;
						NextClosestColor=ResultColor;

						nClosestDistance=nTotalDistSquared;
						ResultColor=
							(nCandRed<<SHIFT_RED)|
							(nCandGreen<<SHIFT_GREEN)|
							(nCandBlue<<SHIFT_BLUE);

					} else if (nTotalDistSquared<nNextClosestDistance) {

						nNextClosestDistance=nTotalDistSquared;
						NextClosestColor=
							(nCandRed<<SHIFT_RED)|
							(nCandGreen<<SHIFT_GREEN)|
							(nCandBlue<<SHIFT_BLUE);

					}

				}

				SPete_ColorReduce_InverseMapEntry* pInvColorMapEntry=
					pInverseColorMap+
					(nBlueIndex<<(cnGridSizeShift*2))+
					(nGreenIndex<<(cnGridSizeShift*1))+
					(nRedIndex<<(cnGridSizeShift*0));

				pInvColorMapEntry->ClosestColor=ResultColor;
				pInvColorMapEntry->NextClosestColor=NextClosestColor;

	    }
	}
    }
	if(nSortedRed  )delete[]nSortedRed;
	if(nSortedGreen)delete[]nSortedGreen;
	if(nSortedBlue )delete[]nSortedBlue;
}
/////////////////////////////////////////////////////////
// static member function
//
/////////////////////////////////////////////////////////
void pix_colorreduce :: obj_setupCallback(t_class *classPtr)
{
  class_addcreator(reinterpret_cast<t_newmethod>(create_pix_colorreduce), 
		   gensym("pix_colourreduce"), A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_colorreduce::countCallback),
		  gensym("count"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_colorreduce::persistCallback),
		  gensym("persist"), A_DEFFLOAT, A_NULL);
    class_addmethod(classPtr, reinterpret_cast<t_method>(&pix_colorreduce::smoothCallback),
		  gensym("smooth"), A_DEFFLOAT, A_NULL);
}
void pix_colorreduce :: countCallback(void *data, t_floatarg m_TargetColorCount)
{
  if(m_TargetColorCount>255)m_TargetColorCount=255.f;
  if(m_TargetColorCount<0)m_TargetColorCount=0.f;
  GetMyClass(data)->m_TargetColorCount=(m_TargetColorCount);
  GetMyClass(data)->setPixModified();
}

void pix_colorreduce :: persistCallback(void *data, t_floatarg m_PalettePersistence)
{
  if(m_PalettePersistence>255)m_PalettePersistence=255.f;
  if(m_PalettePersistence<0)m_PalettePersistence=0.f;
  GetMyClass(data)->m_PalettePersistence=(m_PalettePersistence);
  GetMyClass(data)->setPixModified();
}

void pix_colorreduce :: smoothCallback(void *data, t_floatarg m_BoundarySmoothing)
{
  GetMyClass(data)->m_BoundarySmoothing=!(!static_cast<int>(m_BoundarySmoothing));  
  GetMyClass(data)->setPixModified();
}
