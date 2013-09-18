/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    this port Copyright (c) 2003 James Tittle
    ported from pete's_plugins (www.petewarden.com)
    
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_PIXES_PIX_KALEIDOSCOPE_H_
#define _INCLUDE__GEM_PIXES_PIX_KALEIDOSCOPE_H_

#include "Base/GemPixObj.h"
#include <math.h>

#ifdef __ppc__
#include "Utils/Functions.h"
#undef sqrt
#define sqrt fast_sqrtf
#endif

#define PETE_KALEIDOSCOPE_HALFLINE_BIT		(1<<0)

typedef struct _SPete_Kaleidoscope_Line {
		float X;
		float Y;
		U32 Flags;
} SPete_Kaleidoscope_Line;
extern "C" int Pete_Kaleidoscope_LinesSortFunction(const void* pElem1,const void* pElem2);

#ifdef _WIN32
# define NO_HACK
#endif
/*-----------------------------------------------------------------
-------------------------------------------------------------------
CLASS
    pix_kaleidoscope
    
    View pix thru a variable kaleidoscope

KEYWORDS
    pix
    
DESCRIPTION

    
   
-----------------------------------------------------------------*/
class GEM_EXTERN pix_kaleidoscope : public GemPixObj
{
    CPPEXTERN_HEADER(pix_kaleidoscope, GemPixObj);

    public:

	    //////////
	    // Constructor
    	pix_kaleidoscope();
    	
    protected:
    	
    	//////////
    	// Destructor
    	virtual ~pix_kaleidoscope();

    	//////////
    	// Do the processing
    	virtual void 	processRGBAImage(imageStruct &image);
    	virtual void 	processYUVImage(imageStruct &image);
	virtual void 	processGrayImage(imageStruct &image);


	imageStruct    myImage;

	struct SPete_AngleTable_Entry {
		int nAngleFA;
		int nDist;
	};

	int nWidth;
	int nHeight;
	SPete_MemHandle hAngleTable;
	SPete_MemHandle hCosTable;
	SPete_MemHandle hLines;
	int nMaxLines;
	
	U32*				pSource;
	U32*				pOutput;

	float m_Divisions;
	float m_OutputAnglePreIncrement;
	float m_SourceAnglePreIncrement;
	float m_SourceCentreX;
	float m_SourceCentreY;
	float m_OutputCentreX;
	float m_OutputCentreY;
	float m_ReflectionLineProportion;
	float m_SourceAngleProportion;
			
	int nCosTableSizeShift;// = 10;
	int nCosTableSize;// = (1<<nCosTableSizeShift);

	int nFixedShift;// = 16;
	int nFixedMult;// = (1<<nFixedShift);
	int nFixedMask;// = (nFixedMult-1);

	int nDistanceShift;// = 6;
	int nDistanceMult;// = (1<<nDistanceShift);

	float FixedAngleMult;// = (nFixedMult/Pete_TwoPi);

	int nHalfPiFA;// = (nFixedMult/4);

	int nFAToCosTableShift;// = (nFixedShift - nCosTableSizeShift);

	float Pete_Kaleidoscope_Epsilon;// = 0.0001f;

	int nMaxDivisions;// = 64;
	int init;

	struct SPete_Kaleidoscope_PartitionData {
		SPete_Kaleidoscope_Line* pYNegLines;
		int	nYNegLinesCount;
		SPete_Kaleidoscope_Line* pYPosLines;
		int nYPosLinesCount;
	};
	
	typedef struct SPete_2dMatrix {
		float m[3][3];
	} SPete_2dMatrix;

	typedef struct SPete_2dVector {
	    float x;
	    float y;
	} SPete_2dVector;

	float m_Angle;
	float m_PlaneD;
	float m_DoSimpleMirrorAll;

	inline int Pete_Kaleidoscope_CosFA(int nAngleFA);
	inline int Pete_Kaleidoscope_SinFA(int nAngleFA);
	
	int Pete_Kaleidoscope_Init();
	void Pete_Kaleidoscope_DeInit();
	void Pete_Kaleidoscope_SetupAngleTable();
	void Pete_Kaleidoscope_SetupCosTable();
	void Pete_Kaleidoscope_SetupLines(int* poutLinesCount);
	void Pete_Kaleidoscope_PartitionLines(SPete_Kaleidoscope_Line* pLinesStart,int nLinesCount,SPete_Kaleidoscope_PartitionData* poutPartitionData);
	void Pete_Kaleidoscope_CreateAllTransforms(SPete_2dMatrix* pTransforms);
	void Pete_Kaleidoscope_Dev();
	inline int GetMirrored(int inValue,const int nMax);
	
	inline void Pete_2dVector_Add(SPete_2dVector* pinA,SPete_2dVector* pinB,SPete_2dVector* poutResult)
	{
	    poutResult->x=(pinA->x+pinB->x);
	    poutResult->y=(pinA->y+pinB->y);
	}
	inline void Pete_2dVector_Scale(SPete_2dVector* pinA,float Scale,SPete_2dVector* poutResult)
	{
	    poutResult->x=(pinA->x*Scale);
	    poutResult->y=(pinA->y*Scale);
	}
	inline float Pete_2dVector_DotProduct(SPete_2dVector* pinA,SPete_2dVector* pinB)
	{
	    return (pinA->x*pinB->x)+(pinA->y*pinB->y);
	}
	
	inline void Pete_2dVector_Subtract(SPete_2dVector* pinA,SPete_2dVector* pinB,SPete_2dVector* poutResult) {
	    poutResult->x=(pinA->x-pinB->x);
	    poutResult->y=(pinA->y-pinB->y);
	}
	
	void Pete_2dMatrix_SetToIdentity(SPete_2dMatrix* pMatrix);
	void Pete_2dMatrix_SetToRotation(float Rotation,SPete_2dMatrix* poutResult);
	void Pete_2dMatrix_Concatenate(SPete_2dMatrix* pinFirst,SPete_2dMatrix* pinSecond,SPete_2dMatrix* poutResult);
	void Pete_2dMatrix_SetToDirectionalScale(float NormalX,float NormalY,float Scale,SPete_2dMatrix* poutResult);
	void Pete_2dMatrix_TransformVector(SPete_2dVector* pinVector,SPete_2dMatrix* pinMatrix,SPete_2dVector* poutResult);
	void Pete_2dMatrix_SetToTranslation(float TranslationX,float TranslationY,SPete_2dMatrix* poutResult);
	void Pete_SimpleMirror_Render();

#ifdef NO_HACK
	int* g_pCurrentCosTable; // Pete- Hack to avoid accessing this table via 2 indirections 
#else
	static int* g_pCurrentCosTable; // Pete- Hack to avoid accessing this table via 2 indirections 
#endif

	/* inlets for parameters */
	t_inlet* m_inDiv;
	t_inlet* m_inSAngle;
	t_inlet* m_inSCtr;
	t_inlet* m_inOAngle;
	t_inlet* m_inOCtr;
	t_inlet* m_inRlp;
	t_inlet* m_inSap;
    
    private:
    
    	//////////
    	// Static member functions
        static void 	sourceCtrCallback(void *data, t_floatarg m_SourceCentreX, t_floatarg m_SourceCentreY);
        static void 	outputCtrCallback(void *data, t_floatarg m_OutputCentreX, t_floatarg m_SourceCentreY);
    	static void 	outputAngCallback(void *data, t_floatarg m_OutputAnglePreIncrement);
	static void 	sourceAngCallback(void *data, t_floatarg m_SourceAnglePreIncrement);
    	static void 	outputAngleCallback(void *data, t_floatarg m_OutputAnglePreIncrement);
	static void 	sourceAngleCallback(void *data, t_floatarg m_SourceAnglePreIncrement);
    	static void 	divCallback(void *data, t_floatarg m_Divisions);
	static void 	sapCallback(void *data, t_floatarg m_SourceAngleProportion);
	static void 	rlpCallback(void *data, t_floatarg m_ReflectionLineProportion);
};

#endif	// for header file
