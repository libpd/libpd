/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    GemFuncUtil.h
       - contains functions for graphics
       - part of GEM

    Copyright (c) 1997-2000 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_UTILS_FUNCTIONS_H_
#define _INCLUDE__GEM_UTILS_FUNCTIONS_H_

#ifdef __APPLE__
#include <Carbon/Carbon.h>
#endif

#include "Gem/ExportDef.h"

/* this should be included for ALL platforms:
 * should we define __MMX__ for windows in there ?
 */
#include "Utils/SIMD.h"
#include "Utils/GemMath.h"

// for rand()
#include <stdlib.h>

///////////////////////////////////////////////////////////////////////////////
// powerOfTwo
// get the next higher 2^n-number (if value is'nt 2^n by itself)
///////////////////////////////////////////////////////////////////////////////
inline int powerOfTwo(int value)
{
/*
    int x = 1;
    //    while(x <= value) x <<= 1;
    while(x < value) x <<= 1;
    return(x);
*/
// optimization from "Hacker's Delight"
// - above loop executes in 4n+3 instructions, where n is the power of 2 of returned int
// - below code is branch-free and only 12 instructions!
	value = value - 1;
	value = value | (value >> 1);
	value = value | (value >> 2);
	value = value | (value >> 4);
	value = value | (value >> 8);
	value = value | (value >> 16);
	return (value + 1);
}

///////////////////////////////////////////////////////////////////////////////
// min/max functions
//
///////////////////////////////////////////////////////////////////////////////
#ifndef MIN
inline int MIN(int x, int y) {  return (x<y)?x:y; }
inline float MIN(float x, float y) {  return (x<y)?x:y; }
#endif
#ifndef MAX
inline int MAX(int x, int y) {  return (x>y)?x:y; }
inline float MAX(float x, float y) {  return (x>y)?x:y; }
#endif

inline unsigned char TRI_MAX(unsigned char v1, unsigned char v2, unsigned char v3){
  if (v1 > v2 && v1 > v3) return(v1);
  if (v2 > v3) return(v2);
  return(v3);
}
inline unsigned char TRI_MIN(unsigned char v1, unsigned char v2, unsigned char v3){
  if (v1 < v2 && v1 < v3) return(v1);
  if (v2 < v3) return(v2);
  return(v3);
}

///////////////////////////////////////////////////////////////////////////////
// Clamp functions
//
///////////////////////////////////////////////////////////////////////////////
//////////
// Clamp a value high
inline unsigned char CLAMP_HIGH(int x)
	{ return((unsigned char )((x > 255) ? 255 : x)); }

//////////
// Clamp a value low
inline unsigned char CLAMP_LOW(int x)
	{ return((unsigned char )((x < 0) ? 0 : x)); }

//////////
// Clamp an int to the range of an unsigned char
inline unsigned char CLAMP(int x)
    { return((unsigned char)((x > 255) ? 255 : ( (x < 0) ? 0 : x))); }

//////////
// Clamp a float to the range of an unsigned char
inline unsigned char CLAMP(float x)
    { return((unsigned char)((x > 255.f) ? 255.f : ( (x < 0.f) ? 0.f : x))); }

//////////
// Clamp a float to 0. <= x <= 1.0
inline float FLOAT_CLAMP(float x)
    { return((x > 1.f) ? 1.f : ( (x < 0.f) ? 0.f : x)); }

/////////
// Clamp the Y channel of YUV (16%235)
inline unsigned char CLAMP_Y(int x)
    { return((unsigned char)((x > 235) ? 235 : ( (x < 16) ? 16 : x))); }

///////////////////////////////////////////////////////////////////////////////
// Multiply and interpolation
//
///////////////////////////////////////////////////////////////////////////////
//////////
// Exactly multiply two unsigned chars
// This avoids a float value (important on Intel...)
// From Alvy Ray Smith paper
inline unsigned char INT_MULT(unsigned int a, unsigned int b)
	{ int t = (unsigned int)a * (unsigned int)b + 0x80;
      return((unsigned char)(((t >> 8) + t) >> 8)); }

//////////
// Exactly LERPs two values
// This avoids a float value (important on Intel...)
// From Alvy Ray Smith paper
inline unsigned char INT_LERP(unsigned int p, unsigned int q, unsigned int a)
	{ return((unsigned char)(p + INT_MULT(a, q - p))); }

//////////
// Floating point LERP
inline float FLOAT_LERP(float p, float q, float a)
	{ return( a * (q - p) + p); }


///////////////////////////////////////////////////////////////////////////////
// Step function
//
///////////////////////////////////////////////////////////////////////////////
inline int stepFunc(float x, float a)
    { return(x >= a); }
inline int stepFunc(int x, int a)
    { return(x >= a); }
inline int stepFunc(unsigned char x, unsigned char a)
    { return(x >= a); }

///////////////////////////////////////////////////////////////////////////////
// Pulse function
//
///////////////////////////////////////////////////////////////////////////////
inline int pulseFunc(float x, float a, float b)
    { return(stepFunc(a, x) - stepFunc(b, x)); }
inline int pulseFunc(int x, int a, int b)
    { return(stepFunc(a, x) - stepFunc(b, x)); }
inline int pulseFunc(unsigned char x, unsigned char a, unsigned char b)
    { return(stepFunc(a, x) - stepFunc(b, x)); }


///////////////////////////////////////////////////////////////////////////////
// Clamp function
//
///////////////////////////////////////////////////////////////////////////////
inline float clampFunc(float x, float a, float b)
    { return(x < a ? a : (x > b ? b : x)); }
inline int clampFunc(int x, int a, int b)
    { return(x < a ? a : (x > b ? b : x)); }
inline unsigned char clampFunc(unsigned char x, unsigned char a, unsigned char b)
    { return(x < a ? a : (x > b ? b : x)); }
inline void* clampFunc(void* x, void* a, void* b)
    { return(x < a ? a : (x > b ? b : x)); }
/* 
   inline int GateInt(int nValue,int nMin,int nMax)
   inline float GateFlt(float nValue,float nMin,float nMax)
   inline void* GatePtr(void* pValue,void* pMin,void* pMax)
*/


///////////////////////////////////////////////////////////////////////////////
// absolute integer
//
///////////////////////////////////////////////////////////////////////////////
inline int AbsInt(int inValue)         { return (inValue>0)?inValue:-inValue; }
static inline int GetSign(int inValue) { return (inValue<0)?-1:1;             }

///////////////////////////////////////////////////////////////////////////////
// wrapping functions for integers
//
///////////////////////////////////////////////////////////////////////////////

inline int GetTiled(int inValue,const int nMax) {
  int nOutValue=(inValue%nMax);
  if (nOutValue<0)nOutValue=((nMax-1)+nOutValue);
  return nOutValue;
}

inline int GetMirrored(int inValue,const int nMax) {
  const int nTwoMax=(nMax*2);
  int nOutValue=GetTiled(inValue,nTwoMax);
  if (nOutValue>=nMax)nOutValue=((nTwoMax-1)-nOutValue);
  return nOutValue;
}


///////////////////////////////////////////////////////////////////////////////
// 2D-algebra
//    
///////////////////////////////////////////////////////////////////////////////
static inline void Get2dTangent(float inX,float inY,float* poutX,float* poutY) {
	*poutX=inY;
	*poutY=-inX;
}
///////////////////////////////////////////////////////////////////////////////
// 2D-dot product
///////////////////////////////////////////////////////////////////////////////
static inline float Dot2d(float Ax,float Ay,float Bx,float By) {
	return ((Ax*Bx)+(Ay*By));
}
///////////////////////////////////////////////////////////////////////////////
// 2D-vector normalization
///////////////////////////////////////////////////////////////////////////////
static inline void Normalise2d(float* pX,float* pY) {
	const float MagSqr=Dot2d(*pX,*pY,*pX,*pY);
	float Magnitude=(float)sqrt(MagSqr);
	if (Magnitude<=0.0f) {
		Magnitude=0.001f;
	}
	const float RecipMag=1.0f/Magnitude;

	*pX*=RecipMag;
	*pY*=RecipMag;
}

///////////////////////////////////////////////////////////////////////////////
// higher algebra
//
///////////////////////////////////////////////////////////////////////////////
inline float GetRandomFloat(void) {
  return rand()/static_cast<float>(RAND_MAX);
}


///////////////////////////////////////////////////////////////////////////////
// Smooth step function (3x^2 - 2x^3)
//
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN extern float         smoothStep(float x, float a, float b);
GEM_EXTERN extern int           smoothStep(int x, int a, int b);
GEM_EXTERN extern unsigned char smoothStep(unsigned char x, unsigned char a, unsigned char b);

///////////////////////////////////////////////////////////////////////////////
// Bias function
//
// Remap unit interval (curve)
// If a == 0.5, then is linear mapping.
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN extern float biasFunc(float x, float a);

///////////////////////////////////////////////////////////////////////////////
// Gain function
//
// Remap unit interval (S-curve)
// Will always return 0.5 when x is 0.5
// If a == 0.5, then is linear mapping.
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN extern float gainFunc(float x, float a);

///////////////////////////////////////////////////////////////////////////////
// Linear function
//
// val should be 0 <= val <= 1.
// ret should point at a float of enough dimensions to hold the returned value
//      For instance, numDimen == 2, should have a ret[2]
// numDimen is the number of dimensions to compute
// npnts is the number of points per dimension.
//
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN extern void linearFunc(float val, float *ret, int numDimen, int npnts, float *pnts);

///////////////////////////////////////////////////////////////////////////////
// Spline function
//
// val should be 0 <= val <= 1.
// ret should point at a float of enough dimensions to hold the returned value
//      For instance, numDimen == 2, should have a ret[2]
// numDimen is the number of dimensions to compute
// nknots is the number of knots per dimension.
//      There must be at least four knots!
//
// Thanks to
//      _Texturing and Modeling: A Procedural Approach_
//          David S. Ebert, Ed.
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN extern void splineFunc(float val, float *ret, int numDimen, int nknots, float *knot);


///////////////////////////////////////////////////////////////////////////////
// Pixel access functions
//
///////////////////////////////////////////////////////////////////////////////
//
// Accelerated Pixel Manipulations 
// This is sort on a vector operation on 8 chars at the same time .... could be
// implemented in MMX
// Alpha channel is not added !! (would be nr 3 and 7)

#define ADD8_NOALPHA(a,b) \
 ((unsigned char*)(a))[0] = CLAMP_HIGH((int)((unsigned char*)(a))[0] + ((unsigned char*)(b))[0]);\
 ((unsigned char*)(a))[1] = CLAMP_HIGH((int)((unsigned char*)(a))[1] + ((unsigned char*)(b))[1]);\
 ((unsigned char*)(a))[2] = CLAMP_HIGH((int)((unsigned char*)(a))[2] + ((unsigned char*)(b))[2]);\
 ((unsigned char*)(a))[4] = CLAMP_HIGH((int)((unsigned char*)(a))[4] + ((unsigned char*)(b))[4]);\
 ((unsigned char*)(a))[5] = CLAMP_HIGH((int)((unsigned char*)(a))[5] + ((unsigned char*)(b))[5]);\
 ((unsigned char*)(a))[6] = CLAMP_HIGH((int)((unsigned char*)(a))[6] + ((unsigned char*)(b))[6]);\

#define SUB8_NOALPHA(a,b) \
 ((unsigned char*)(a))[0] = CLAMP_LOW((int)((unsigned char*)(a))[0] - ((unsigned char*)(b))[0]);\
 ((unsigned char*)(a))[1] = CLAMP_LOW((int)((unsigned char*)(a))[1] - ((unsigned char*)(b))[1]);\
 ((unsigned char*)(a))[2] = CLAMP_LOW((int)((unsigned char*)(a))[2] - ((unsigned char*)(b))[2]);\
 ((unsigned char*)(a))[4] = CLAMP_LOW((int)((unsigned char*)(a))[4] - ((unsigned char*)(b))[4]);\
 ((unsigned char*)(a))[5] = CLAMP_LOW((int)((unsigned char*)(a))[5] - ((unsigned char*)(b))[5]);\
 ((unsigned char*)(a))[6] = CLAMP_LOW((int)((unsigned char*)(a))[6] - ((unsigned char*)(b))[6]);\

#define ADD8(a,b) \
 ((unsigned char*)(a))[0] = CLAMP_HIGH((int)((unsigned char*)(a))[0] + ((unsigned char*)(b))[0]);\
 ((unsigned char*)(a))[1] = CLAMP_HIGH((int)((unsigned char*)(a))[1] + ((unsigned char*)(b))[1]);\
 ((unsigned char*)(a))[2] = CLAMP_HIGH((int)((unsigned char*)(a))[2] + ((unsigned char*)(b))[2]);\
 ((unsigned char*)(a))[3] = CLAMP_HIGH((int)((unsigned char*)(a))[3] + ((unsigned char*)(b))[3]);\
 ((unsigned char*)(a))[4] = CLAMP_HIGH((int)((unsigned char*)(a))[4] + ((unsigned char*)(b))[4]);\
 ((unsigned char*)(a))[5] = CLAMP_HIGH((int)((unsigned char*)(a))[5] + ((unsigned char*)(b))[5]);\
 ((unsigned char*)(a))[6] = CLAMP_HIGH((int)((unsigned char*)(a))[6] + ((unsigned char*)(b))[6]);\
 ((unsigned char*)(a))[7] = CLAMP_HIGH((int)((unsigned char*)(a))[7] + ((unsigned char*)(b))[7]);\

#define SUB8(a,b) \
 ((unsigned char*)(a))[0] = CLAMP_LOW((int)((unsigned char*)(a))[0] - ((unsigned char*)(b))[0]);\
 ((unsigned char*)(a))[1] = CLAMP_LOW((int)((unsigned char*)(a))[1] - ((unsigned char*)(b))[1]);\
 ((unsigned char*)(a))[2] = CLAMP_LOW((int)((unsigned char*)(a))[2] - ((unsigned char*)(b))[2]);\
 ((unsigned char*)(a))[3] = CLAMP_LOW((int)((unsigned char*)(a))[3] - ((unsigned char*)(b))[3]);\
 ((unsigned char*)(a))[4] = CLAMP_LOW((int)((unsigned char*)(a))[4] - ((unsigned char*)(b))[4]);\
 ((unsigned char*)(a))[5] = CLAMP_LOW((int)((unsigned char*)(a))[5] - ((unsigned char*)(b))[5]);\
 ((unsigned char*)(a))[6] = CLAMP_LOW((int)((unsigned char*)(a))[6] - ((unsigned char*)(b))[6]);\
 ((unsigned char*)(a))[7] = CLAMP_LOW((int)((unsigned char*)(a))[7] - ((unsigned char*)(b))[7]);\



#ifdef __APPLE__
//Ian Ollman's function to determine the cache prefetch for altivec vec_dst()
inline UInt32 GetPrefetchConstant( int blockSizeInVectors, int blockCount, int blockStride )
{
	return ((blockSizeInVectors << 24) & 0x1F000000) | ((blockCount << 16) & 0x00FF0000) | (blockStride & 0xFFFF);
} 
#endif


#endif  // for header file

