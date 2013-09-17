/* 
 * sgn~: sign of a signal
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "zexySIMD.h"

typedef struct _sgnTilde
{
  t_object x_obj;
} t_sgnTilde;


/* ------------------------ sgn~ ----------------------------- */

static t_class *sgnTilde_class;

static t_int *sgnTilde_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);
  t_sample x;
  while (n--) {
    x=*in++;
    if (x>0.) *out++=1.;
    else if	(x<0.) *out++=-1.;
    else *out++=0.;
  }
  
  return (w+4);
}
static t_int *sgnTilde_perform8(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3])>>3;
  t_sample x;

  while(n--){
    /* weirdly enough, the if/else/if/else is a lot faster than ()?:(()?:) */
    if ((x=in[0])>0.) out[0]=1.; else if(x<0.) out[0]=-1.; else out[0]=0.;
    if ((x=in[1])>0.) out[1]=1.; else if(x<0.) out[1]=-1.; else out[1]=0.;
    if ((x=in[2])>0.) out[2]=1.; else if(x<0.) out[2]=-1.; else out[2]=0.;
    if ((x=in[3])>0.) out[3]=1.; else if(x<0.) out[3]=-1.; else out[3]=0.;
    if ((x=in[4])>0.) out[4]=1.; else if(x<0.) out[4]=-1.; else out[4]=0.;
    if ((x=in[5])>0.) out[5]=1.; else if(x<0.) out[5]=-1.; else out[5]=0.;
    if ((x=in[6])>0.) out[6]=1.; else if(x<0.) out[6]=-1.; else out[6]=0.;
    if ((x=in[7])>0.) out[7]=1.; else if(x<0.) out[7]=-1.; else out[7]=0.;

    in+=8;
    out+=8;
  }
  
  return (w+4);
}

#ifdef __SSE__
static int l_bitmask[]={0x80000000, 0x80000000, 0x80000000, 0x80000000}; /* sign bitmask */
static t_int *sgnTilde_performSSE(t_int *w)
{
  __m128 *in = (__m128 *)(w[1]);
  __m128 *out = (__m128 *)(w[2]);

  __m128 val;
  int n = (int)(w[3])>>3; /* we do 8x loop-unrolling */

  const __m128 sgnmask= _mm_loadu_ps((float*)l_bitmask);
  const __m128 zero   = _mm_setzero_ps();
  const __m128 one    = _mm_set_ps(1.f, 1.f, 1.f, 1.f);

  __m128 xmm0, xmm1;

  while (n--) {

    val=in[0];
    xmm0  = _mm_cmpneq_ps(val , zero);/* mask for non-zeros */
    xmm1  = _mm_and_ps   (val, sgnmask);/* sign (without value) */
    xmm0  = _mm_and_ps   (xmm0, one); /* (abs) value: (val==0.f)?0.f:1.f */
    out[0]= _mm_or_ps    (xmm1, xmm0);/* merge sign and value */

    val=in[1];
    xmm0  = _mm_cmpneq_ps(val , zero);
    xmm1  = _mm_and_ps   (val, sgnmask);
    xmm0  = _mm_and_ps   (xmm0, one);
    out[1]= _mm_or_ps    (xmm1, xmm0);

    in +=2;
    out+=2;

  }
  return (w+4);
}
#endif /* __SSE__ */

static void sgnTilde_dsp(t_sgnTilde *x, t_signal **sp)
{
#ifdef __SSE__
  if(
     ZEXY_TYPE_EQUAL(t_sample, float) && /*  currently SSE2 code is only for float (not for double) */
     zexy_testSSE(sgnTilde_perform,
		  sgnTilde_performSSE, 
		  1,1)
     )
    {
      dsp_add(sgnTilde_performSSE, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    } else
#endif
    if (sp[0]->s_n & 7) {
      dsp_add(sgnTilde_perform , 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    } else {
      dsp_add(sgnTilde_perform8, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
    }

}

static void sgnTilde_helper(void)
{
  post("\n%c sgn~ \t\t:: sign of a signal", HEARTSYMBOL);
}

static void *sgnTilde_new(void)
{
  t_sgnTilde *x = (t_sgnTilde *)pd_new(sgnTilde_class);
  outlet_new(&x->x_obj, gensym("signal"));
  
  return (x);
}

void sgn_tilde_setup(void)
{
  sgnTilde_class = class_new(gensym("sgn~"), (t_newmethod)sgnTilde_new, 0,
                             sizeof(t_sgnTilde), 0, A_DEFFLOAT, 0);
  class_addmethod(sgnTilde_class, nullfn, gensym("signal"), 0);
  class_addmethod(sgnTilde_class, (t_method)sgnTilde_dsp, gensym("dsp"), 0);
  
  class_addmethod(sgnTilde_class, (t_method)sgnTilde_helper, gensym("help"), 0);
  class_sethelpsymbol(sgnTilde_class, gensym("zigbinops"));
  zexy_register("sgn~");
}
