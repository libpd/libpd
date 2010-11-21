#ifndef __M_FIXED_H__
#define __M_FIXED_H__

#ifdef FIXEDPOINT
typedef int t_sample;
#endif

#define t_fixed int
#define fix1 18  /* (18) number of bits after comma */


#define fixfac ((float)(1<<fix1))  /* float factor (for scaling ftofix ..) */


/* fixed point multiplication and division */

#define mult(a,b) (long long)(((long long) (a) * (long long) (b))>>fix1)
#define idiv(a,b) ((((long long) (a) )<<fix1)/(long long) (b) )

/* conversion macros */

#define itofix(a) ((a) << fix1)
#define ftofix(a) ((t_fixed)( (a) *(double)fixfac + 0.5))

#define fixtof(a) ((double) (a) * 1./(fixfac-0.5))
#define fixtoi(a) ((a) >>fix1)


/* Not working !! */

#define fnum(a) ( (a) >>(fix1-16))
#define ffrac(a) (0)


/* mapping of fft functions */

#ifdef FIXEDPOINT
#define mayer_realifft imayer_realifft
#define mayer_realfft imayer_realfft
#define mayer_fft imayer_fft
#define mayer_ifft imayer_ifft
#endif

#ifdef FIXEDPOINT
#define SCALE16(x) (x>>(fix1-15))
#define SCALE32(x) (x<<(32-fix1))
#define INVSCALE16(x) (x<<(fix1-15))
#else
#define SCALE16(x) (32767.*x)
#define SCALE32(x) (2147483648.*x)
#define INVSCALE16(x) ((float)3.051850e-05*x)
#endif


#endif


