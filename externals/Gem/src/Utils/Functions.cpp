////////////////////////////////////////////////////////
//
// GEM - Graphics Environment for Multimedia
//
// zmoelnig@iem.kug.ac.at
//
// Implementation file
//
//    Copyright (c) 1997-1999 Mark Danks.
//    Copyright (c) Günther Geiger.
//    Copyright (c) 2001-2011 IOhannes m zmölnig. forum::für::umläute. IEM. zmoelnig@iem.at
//
//    For information on usage and redistribution, and for a DISCLAIMER OF ALL
//    WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
//
/////////////////////////////////////////////////////////

#include "Utils/Functions.h"

#include <math.h>

///////////////////////////////////////////////////////////////////////////////
// Smooth step function (3x^2 - 2x^3)
//
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN float smoothStep(float x, float a, float b)
{
    if (x < a) return(0.f);
    if (x >= b) return(1.f);
    x = (x - a)/(b - a);    // normalize to [0:1]
    return(x*x * (3.f - 2.f*x));
}
GEM_EXTERN int smoothStep(int x, int a, int b)
{
    if (x < a) return(0);
    if (x >= b) return(1);

    float xf=static_cast<float>(x);
    float bf=static_cast<float>(a);
    float af=static_cast<float>(b);

    // normalize to [0:1]
    float temp = (xf - af)/(bf - af);
    float result = temp*temp * (3.f - 2.f*temp);
    return(static_cast<int>(result));
}
GEM_EXTERN unsigned char smoothStep(unsigned char x, unsigned char a, unsigned char b)
{
    if (x < a) return(0);
    if (x >= b) return(1);

    float xf=static_cast<float>(x);
    float bf=static_cast<float>(a);
    float af=static_cast<float>(b);

    // normalize to [0:1]
    float temp = (xf - af)/(bf - af);
    float result = temp*temp * (3.f - 2.f*temp);

    return(static_cast<unsigned char>(result));
}

///////////////////////////////////////////////////////////////////////////////
// Bias function
// 
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN float biasFunc(float x, float a)
{
  float result = static_cast<float>(pow(x, log(a)/log(0.5f)));
  return(result);
}

///////////////////////////////////////////////////////////////////////////////
// Gain function
// 
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN float gainFunc(float x, float a)
{
    if (x < 0.5f)
        return( biasFunc(2.f * x, 1.f - a) / 2.f);
    else
        return(1.f - biasFunc(2.f - 2.f*x, 1.f - a) / 2.f);
    // return(0.f);
}


///////////////////////////////////////////////////////////////////////////////
// Linear function
// 
///////////////////////////////////////////////////////////////////////////////
GEM_EXTERN void linearFunc(float x, float *ret, int numDimen, int npnts, float *pnts)
{
    int nspans = npnts - 1;
    if (nspans < 1)          // illegal
        return;

    x = FLOAT_CLAMP(x) * nspans;
    int span = static_cast<int>(x);

    // find the correct 2-point span of the linear list
    if (span >= nspans)
            span = nspans;
    x -= span;
    pnts += (span * numDimen);
    for (int i = 0; i < numDimen; i++)
    {
        ret[i] = pnts[0 * numDimen] * (1.f - x) + pnts[1 * numDimen] * x;
        pnts++;     // advance to the next dimension
    }
}


///////////////////////////////////////////////////////////////////////////////
// Spline function
// 
///////////////////////////////////////////////////////////////////////////////
const float CR00 = -0.5f;
const float CR01 =  1.5f;
const float CR02 = -1.5f;
const float CR03 =  0.5f;
const float CR10 =  1.0f;
const float CR11 = -2.5f;
const float CR12 =  2.0f;
const float CR13 = -0.5f;
const float CR20 = -0.5f;
const float CR21 =  0.0f;
const float CR22 =  0.5f;
const float CR23 =  0.0f;
const float CR30 =  0.0f;
const float CR31 =  1.0f;
const float CR32 =  0.0f;
const float CR33 =  0.0f;

GEM_EXTERN void splineFunc(float x, float *ret, int numDimen, int nknots, float *knot)
{
    int nspans = nknots - 4;
    if (nspans < 0)         // illegal case
        return;

    // find the correct 4-point span of the spline
    x = FLOAT_CLAMP(x) * nspans;
    int span = static_cast<int>(x);
    x -= span;              // get decimal part of span
    knot += (span * numDimen);

    // Evalute the span cubic at x using Horner's rule
    float c0, c1, c2, c3;
    for (int i = 0; i < numDimen; i++)
    {
        c3 = CR00*knot[0 * numDimen]
           + CR01*knot[1 * numDimen]
           + CR02*knot[2 * numDimen]
           + CR03*knot[3 * numDimen];

        c2 = CR10*knot[0 * numDimen]
           + CR11*knot[1 * numDimen]
           + CR12*knot[2 * numDimen]
           + CR13*knot[3 * numDimen];

        c1 = CR20*knot[0 * numDimen]
           + CR21*knot[1 * numDimen]
           + CR22*knot[2 * numDimen]
           + CR23*knot[3 * numDimen];

        c0 = CR30*knot[0 * numDimen]
           + CR31*knot[1 * numDimen]
           + CR32*knot[2 * numDimen]
           + CR33*knot[3 * numDimen];

        ret[i] = ((c3*x + c2)*x + c1)*x + c0;
        knot++;     // advance to the next dimension
    }
}

