
/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

    include file for SIMD

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

/* 
 * compiler-issues:
 *
 * gcc: when gcc is invoked with "-mmmx" (or "-msse2" or "-maltivec") 
 *      the defines __MMX__ (or corresponding) will be defined automatically
 *
 * vc6: you will have to install the microsoft processor-pack to use MMX/SSE2
 *      you have to have the sp5 for vc6 installed (note: do not install sp6!!)
 * vc6/vc7: (i think) you need to define __MMX__ (and friends) by hand
 */

#ifndef _INCLUDE__GEM_UTILS_SIMD_H_
#define _INCLUDE__GEM_UTILS_SIMD_H_

#define GEM_VECTORALIGNMENT 128

const int GEM_SIMD_NONE=0;
const int GEM_SIMD_MMX=1;
const int GEM_SIMD_SSE2=2;
const int GEM_SIMD_ALTIVEC=3;


#if defined __APPLE__ && defined __VEC__
# ifndef __APPLE_ALTIVEC__
#  undef __VEC__
# endif
#endif

/* include for SIMD on PC's */
#ifdef __SSE2__
#include <emmintrin.h>
// for icc this should be <dvec.h>
typedef union{
  unsigned char c[16];
  __m128i v;
} vector_128;
#elif defined __VEC__
/* for AltiVec (PowerPC) */
typedef union{
  unsigned char c[16];
  vector unsigned char v;
} vector_128;
#endif

#if defined __MMX__
# include <mmintrin.h>
// for icc this should be <ivec.h>
typedef union{
  __m64 v; unsigned char c[8];
} vector64i;

#endif


#ifdef __SSE__
#include <xmmintrin.h>

typedef union{
  __m128 m; float f[4];
} vector128f;
#endif

#include "Gem/ExportDef.h"


/* this is a help-class to query the capabilities of the cpu
 * whenever you want to use SIMD-optimized code, you should
 * make sure you chose the code-block based on the "cpuid" value
 * of this class and NOT simply on preprocessor defines.
 *
 * this class needs only be instantiated once (and it is done in GemMan)
 * this sets the cpuid
 */
class GEM_EXTERN GemSIMD
{
 public:
   GemSIMD(void);
  virtual ~GemSIMD(void);

  /* this gets the "cpuid" (something like GEM_SIMD_NONE) */
  static int getCPU(void);

  /* change the cpuid returned by getCPU()
   * you can only set the cpuid to something that is actually supported
   * by your processor
   */
  static int requestCPU(int cpuid);

  /* performs a runtime-check (if possible) to determine the capabilities
   * of the CPU
   * sets realcpuid appropriately and returns this  value
   */
  static int simd_runtime_check(void);

 private:
  /* this is the maximum capability of the CPU */
  static int realcpuid;
  /* this is the current choosen capability (normally this equals realcpuid) */
  static int cpuid;
};

#endif /* _INCLUDE__GEM_UTILS_SIMD_H_ */
