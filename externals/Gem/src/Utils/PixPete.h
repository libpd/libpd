/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    helper stuff for ports of Pete's Plugins

    http://www.petewarden.com

    Copyright (c) 2004-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/
#ifndef _INCLUDE__GEM_UTILS_PIXPETE_H_
#define _INCLUDE__GEM_UTILS_PIXPETE_H_

#include <stdlib.h>

// utility functions from PeteHelpers.h

#ifndef _MSC_VER
# include <stdint.h>
typedef uint32_t U32; 
typedef uint16_t U16;
typedef uint8_t U8;
#else
typedef unsigned long U32; 
typedef unsigned short U16;
typedef unsigned char U8;
#endif

/* is this only on my system ?
   i thought Gem's YUV is UYVY and not YVYU
   seems weird... (jmz)
*/
#ifdef __APPLE__

# define SHIFT_ALPHA	(24)
# define SHIFT_RED	(16)
# define SHIFT_GREEN	(8)
# define SHIFT_BLUE	(0)

# define SHIFT_U  (24)
# define SHIFT_Y1 (16)
# define SHIFT_V  (8)
# define SHIFT_Y2 (0)

#else

# define SHIFT_ALPHA	(24)
# define SHIFT_RED	(16)
# define SHIFT_GREEN	(8)
# define SHIFT_BLUE	(0)

# define SHIFT_U  (0)
# define SHIFT_Y1 (8)
# define SHIFT_V  (16)
# define SHIFT_Y2 (24)
#endif

const float Pete_Pi=3.141582f;
const float Pete_TwoPi=(2.0f*Pete_Pi);
const float Pete_HalfPi=(0.5f*Pete_Pi);

static inline void Pete_ZeroMemory(void* pMemory,int nCount) {
	char* pCurrent=(char*)pMemory;
	char* pEnd=(pCurrent+nCount);
	//	while (pCurrent<pEnd)	*pCurrent=0;
	//		pCurrent+=1;
	//	}
	while(pCurrent<pEnd)*pCurrent++=0;
//	memset(pMemory,0,nCount);
}

typedef U32		PETE_PIXELDATA32;
#define SIZEOF_PETE_PIXELDATA32 (4)

typedef U32		PETE_PIXELDATA24;
#define SIZEOF_PETE_PIXELDATA24 (3)

typedef U16		PETE_PIXELDATA16;
#define SIZEOF_PETE_PIXELDATA16 (2)

static inline void Pete_CopyAndConvert24BitTo32Bit(PETE_PIXELDATA24* pSource,PETE_PIXELDATA32* pOutput,int nPixelCount) {

	char* pSourceEnd=((char*)pSource)+(nPixelCount*SIZEOF_PETE_PIXELDATA24);
	char* pCurrentSource=((char*)pSource);
	char* pCurrentOutput=((char*)pOutput);

	while (pCurrentSource<pSourceEnd) {
		*((PETE_PIXELDATA32*)pCurrentOutput)=
		*((PETE_PIXELDATA24*)pCurrentSource);

		pCurrentSource+=SIZEOF_PETE_PIXELDATA24;
		pCurrentOutput+=SIZEOF_PETE_PIXELDATA32;
	}
}

static inline void Pete_CopyAndConvert32BitTo24Bit(PETE_PIXELDATA32* pSource,PETE_PIXELDATA24* pOutput,int nPixelCount) {

	char* pSourceEnd=((char*)pSource)+(nPixelCount*SIZEOF_PETE_PIXELDATA32);
	char* pCurrentSource=((char*)pSource);
	char* pCurrentOutput=((char*)pOutput);

	while (pCurrentSource<pSourceEnd) {
		*((PETE_PIXELDATA24*)pCurrentOutput)=
		*((PETE_PIXELDATA32*)pCurrentSource);

		pCurrentSource+=SIZEOF_PETE_PIXELDATA32;
		pCurrentOutput+=SIZEOF_PETE_PIXELDATA24;
	}
}

static inline void Pete_InPlaceConvert24BitTo32Bit(PETE_PIXELDATA24* pBuffer,int nPixelCount) {
	char* pBufferStart=(char*)pBuffer;
	
	char* pBuffer32Current=(pBufferStart+((nPixelCount-1)*SIZEOF_PETE_PIXELDATA32));
	char* pBuffer24Current=(pBufferStart+((nPixelCount-1)*SIZEOF_PETE_PIXELDATA24));

	while (pBuffer32Current>=pBufferStart) {

		*((PETE_PIXELDATA32*)pBuffer32Current)=
		*((PETE_PIXELDATA24*)pBuffer24Current);

		pBuffer32Current-=SIZEOF_PETE_PIXELDATA32;
		pBuffer24Current-=SIZEOF_PETE_PIXELDATA24;
	}
}

static inline void Pete_CopyAndConvert16Bit565To32Bit(PETE_PIXELDATA16* pSource,PETE_PIXELDATA32* pOutput,int nPixelCount) {

	char* pSourceEnd=((char*)pSource)+(nPixelCount*SIZEOF_PETE_PIXELDATA16);
	char* pCurrentSource=((char*)pSource);
	char* pCurrentOutput=((char*)pOutput);

	while (pCurrentSource<pSourceEnd) {

		PETE_PIXELDATA16 SourceColour=
			*((PETE_PIXELDATA16*)pCurrentSource);

		const int nMaskedRed=(SourceColour>>11)&31;
		const int nMaskedGreen=(SourceColour>>5)&63;
		const int nMaskedBlue=(SourceColour>>0)&31;

		const int nNormalizedRed=(nMaskedRed<<3)|(nMaskedRed>>2);
		const int nNormalizedGreen=(nMaskedGreen<<2)|(nMaskedGreen>>4);
		const int nNormalizedBlue=(nMaskedBlue<<3)|(nMaskedBlue>>2);

		const PETE_PIXELDATA32 OutputColour=
			(nNormalizedRed<<16)|
			(nNormalizedGreen<<8)|
			(nNormalizedBlue<<0);

		*((PETE_PIXELDATA32*)pCurrentOutput)=OutputColour;

		pCurrentSource+=SIZEOF_PETE_PIXELDATA16;
		pCurrentOutput+=SIZEOF_PETE_PIXELDATA32;

	}

}

static inline void Pete_CopyAndConvert32BitTo16Bit565(PETE_PIXELDATA32* pSource,PETE_PIXELDATA16* pOutput,int nPixelCount) {

	char* pSourceEnd=((char*)pSource)+(nPixelCount*SIZEOF_PETE_PIXELDATA32);
	char* pCurrentSource=((char*)pSource);
	char* pCurrentOutput=((char*)pOutput);

	while (pCurrentSource<pSourceEnd) {

		PETE_PIXELDATA32 SourceColour=
			*((PETE_PIXELDATA32*)pCurrentSource);

		const int nSourceRed=(SourceColour>>16)&0xff;
		const int nSourceGreen=(SourceColour>>8)&0xff;
		const int nSourceBlue=(SourceColour>>0)&0xff;

		const int nMaskedRed=(nSourceRed>>3);
		const int nMaskedGreen=(nSourceGreen>>2);
		const int nMaskedBlue=(nSourceBlue>>3);

		PETE_PIXELDATA16 OutputColour=
			(nMaskedRed<<11)|
			(nMaskedGreen<<5)|
			(nMaskedBlue<<0);

		*((PETE_PIXELDATA16*)pCurrentOutput)=OutputColour;

		pCurrentSource+=SIZEOF_PETE_PIXELDATA32;
		pCurrentOutput+=SIZEOF_PETE_PIXELDATA16;
	}
}
typedef void* SPete_MemHandle;

inline SPete_MemHandle Pete_NewHandle(int nBytesToAlloc) {
	return malloc(nBytesToAlloc);
}
	
inline void Pete_FreeHandle(SPete_MemHandle InHandle) {
	free(InHandle);
}

inline void* Pete_LockHandle(SPete_MemHandle InHandle) {
	return InHandle;
}

inline void Pete_UnLockHandle(SPete_MemHandle InHandle) {
	// do nothing
}

const int cnBiggestSignedInt=0x7fffffff;

inline int GetLuminance(const U32 inColour) {
	const int nRed=(inColour&(0xff<<SHIFT_RED))>>16;
	const int nGreen=(inColour&(0xff<<SHIFT_GREEN))>>8;
	const int nBlue=(inColour&(0xff<<SHIFT_BLUE))>>0;

	const int nLuminance =
            ((77 * nRed)+
             (150* nGreen)+ // used to be 50 which is plain wrong
             (29 * nBlue));

	return nLuminance;
}

#endif /* _INCLUDE__GEM_UTILS_PIXPETE_H_ */
// end of PeteHelpers.h stuff
