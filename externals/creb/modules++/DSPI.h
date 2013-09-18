
#include "m_pd.h"

#ifndef DSPI_h
#define DSPI_h

#define DSPImin(x,y)			(((x)<(y)) ? (x) : (y))
#define DSPImax(x,y)			(((x)>(y)) ? (x) : (y))
#define DSPIclip(min, x, max)	(DSPImin(DSPImax((min), (x)), max))


// test if floating point number is denormal

#if defined(__i386__) || defined(__x86_64__) // Type punning code:

#ifndef PD_FLOAT_PRECISION
#define PD_FLOAT_PRECISION 32
#endif

#if PD_FLOAT_PRECISION == 32

typedef union
{
    unsigned int i;
    t_float f;
} t_dspiflint;

static inline int DSPI_IS_DENORMAL(t_float f) 
{
    t_dspiflint pun;
    pun.f = f;
    return ((pun.i & 0x7f800000) == 0);
}

// test if almost denormal, choose whichever is fastest
static inline int DSPI_IS_ALMOST_DENORMAL(t_float f) 
{
    t_dspiflint pun;
    pun.f = f;
    return ((pun.i & 0x7f800000) < 0x08000000);
}

#elif PD_FLOAT_PRECISION == 64

typedef union
{
    unsigned int i[2];
    t_float f;
} t_dspiflint;

static inline int DSPI_IS_DENORMAL(t_float f) 
{
    t_dspiflint pun;
    pun.f = f;
    return ((pun.i[1] & 0x7ff00000) == 0);
}

static inline int DSPI_IS_ALMOST_DENORMAL(t_float f) 
{
    t_dspiflint pun;
    pun.f = f;
    return ((pun.i[1] & 0x7ff00000) < 0x10000000);
}

#endif // endif PD_FLOAT_PRECISION
#else   // if not defined(__i386__) || defined(__x86_64__)
#define DSPI_IS_DENORMAL(f) 0
#endif // end if defined(__i386__) || defined(__x86_64__)


//#define DSPI_IS_ALMOST_DENORMAL(f) (fabs(f) < 3.e-34) 

#endif // end ifndef DSPI_h
