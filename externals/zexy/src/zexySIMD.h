#include "zexy.h"

#ifdef __SSE__

#include <xmmintrin.h>
#define Z_SIMD_BLOCK 16  /* must be a power of 2 */
#define Z_SIMD_BYTEALIGN (128/8)   /* assume 128 bits */
#define Z_SIMD_CHKBLOCKSIZE(n) (!(n&(Z_SIMD_BLOCK-1)))
#define Z_SIMD_CHKALIGN(ptr) ( ((unsigned long)(ptr) & (Z_SIMD_BYTEALIGN-1)) == 0 )

typedef union {
  __m128 vec;
  t_sample f[4];
} t_sample4;

/**
 * runs a check whether the SSE-optimized perform routine returns the same result as the generic routine
 * if the results differ, the SSE-code is probably broken, so we should fallback to the generic code
 */
static int zexy_testSSE(t_perfroutine genericperf, t_perfroutine sseperf, unsigned int numinchannels, unsigned int numoutchannels)
{
/* this currently only works with single input, single output */
/* LATER make it work truely multichannel */
  if(1==numinchannels && 1==numoutchannels) {
    t_int w1[4], w2[4];
    t_sample4 in, in1[4], in2[4], out1[4], out2[4];
    int i, j;

    z_verbose(2, "checking for SSE compatibility");

    in.f[0]=0.;
    in.f[1]=-0.5;
    in.f[2]=0.5;
    in.f[1]=5.;

    for(i=0; i<4; i++) {
      in1[i].f[0]=in.f[i]; in1[i].f[1]=in.f[i]; in1[i].f[3]=in.f[i]; in1[i].f[2]=in.f[i];
      out1[i].f[0]=out1[i].f[1]=out1[i].f[2]=out1[i].f[3]=0.f;

      in2[i].f[0]=in.f[i]; in2[i].f[1]=in.f[i]; in2[i].f[3]=in.f[i]; in2[i].f[2]=in.f[i];
      out2[i].f[0]=out2[i].f[1]=out2[i].f[2]=out2[i].f[3]=0.f;
    }

    w1[0]=(t_int)0; w1[1]=(t_int)&in1; w1[2]=(t_int)&out1; w1[3]=(t_int)16; (*genericperf)(w1);
    w2[0]=(t_int)0; w2[1]=(t_int)&in2; w2[2]=(t_int)&out2; w2[3]=(t_int)16; (*sseperf)(w2);


    for(i=0; i<4; i++) {
      for(j=0; j<4; j++) {
	if(fabsf(out1[i].f[j]-out2[i].f[j])>1e-17) {
	  z_verbose(2, "generic and optimized routines return different results: skipping optimization");
	  z_verbose(2, "[%d,%d]: ((%f->%f)!=(%f->%f))",
		    i, j,
		    in1[i].f[j], out1[i].f[j],
		    in2[i].f[j], out2[i].f[j]
		    );
	  return 0;
	}
      }
    }
  } else {
    /* no tests yet */
  }
  z_verbose(2, "using SSE optimization");
  return 1;
}

#endif /* __SSE__ */


