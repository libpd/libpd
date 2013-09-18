/*-----------------------------------------------------------------
LOG
    GEM - Graphics Environment for Multimedia

	Matrix class

    Copyright (c) 1997-1999 Mark Danks. mark@danks.org
    Copyright (c) Günther Geiger. geiger@epy.co.at
    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
    For information on usage and redistribution, and for a DISCLAIMER OF ALL
    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.

-----------------------------------------------------------------*/

#ifndef _INCLUDE__GEM_UTILS_GEMMATH_H_
#define _INCLUDE__GEM_UTILS_GEMMATH_H_
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifdef __APPLE__
# include <AvailabilityMacros.h>
# if defined (MAC_OS_X_VERSION_10_3) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_3
# else

#define sqrtf(v) (float)sqrt((double)(v))
#define cosf(v)  (float)cos((double)(v))
#define sinf(v)  (float)sin((double)(v))
#define tanf(v)  (float)tan((double)(v))
#define logf(v)  (float)log((double)(v))
#define expf(v)  (float)exp((double)(v))

#define atan2f(v,p) (float)atan2((double)(v), (double)(p))
#define powf(v,p)   (float)pow((double)(v), (double)(p))

# endif /* OSX_10_3 */
#endif /* __APPLE__ */


///////////////////////////////////////////////////////////////////////////////
//  Speedup found via Shark:  ppc only
//
//   If you do not require full precision, you can use the PowerPC floating-point 
// reciprocal square-root estimate instruction (frsqrte) instead of calling sqrt().
//
//   If needed, you can increase the precision of the estimate returned by 
// frsqrte (5-bits of precision) by using the Newton-Raphson method for improving 
// the estimate (x0) for 1/sqrt(a) (x1 = 0.5 * x0 * [3.0 - a * x0 * x0]).
///////////////////////////////////////////////////////////////////////////////
#ifdef __ppc__
# include <ppc_intrinsics.h>
# ifdef sqrt
#  undef sqrt
# endif
# ifdef sqrtf
#  undef sqrtf
# endif

# define sqrt fast_sqrtf
# define sqrtf fast_sqrtf

inline double fast_sqrt(double x)
{
	register double est = __frsqrte(x);
	return x * 0.5 * est * __fnmsub(est * est, x, 3.0);
}

inline float fast_sqrtf(float x)
{
	register float est = (float)__frsqrte(x);
	return x * 0.5f * est * __fnmsubs(est * est, x, 3.0f);
}
#endif /* __ppc__ */

#ifdef _WIN32
/* seems like there is no drand48() on w32 */
/* JMZ: this should really return "double" instead of "float",
 * but we need only float... */
# define drand48() ((float)rand())/((float)RAND_MAX)
#endif /* _WIN32 */


#endif	// for header file
