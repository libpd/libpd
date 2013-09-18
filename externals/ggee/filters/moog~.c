/* (C) Guenter Geiger <geiger@epy.co.at> */


#include "math.h"
#include <m_pd.h>

/* ----------------------------- moog ----------------------------- */
static t_class *moog_class;


typedef struct _moog
{
     t_object x_obj;
  t_pd   in2;
     t_float x_1,x_2,x_3,x_4;
     t_float y_1,y_2,y_3,y_4;
} t_moog;

static void moog_reset(t_moog *x)
{
	x->x_1 = x->x_2 = x->x_3 = x->x_4 = 0.0;
	x->y_1 = x->y_2 = x->y_3 = x->y_4 = 0.0;

}



static void *moog_new(t_symbol *s, int argc, t_atom *argv)
{
    if (argc > 1) post("moog~: extra arguments ignored");
    {
	t_moog *x = (t_moog *)pd_new(moog_class);
	outlet_new(&x->x_obj, &s_signal);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	inlet_new(&x->x_obj, &x->in2, &s_signal, &s_signal);
	moog_reset(x);
	return (x);
    }


}



static t_float calc_k(t_float f,t_float k) {
  if (k>4.) k =4.;
  if (k < 0.) k = 0.;
  if (f <= 3800) return k;
  k = k - 0.5*((f-3800)/4300);
  return k;
}

t_int *moog_perform(t_int *w)
{
    t_moog* x = (t_moog*) (w[1]); 
    t_float *in1 = (t_float *)(w[2]);
    t_float *p = (t_float *)(w[3]);
    t_float *k = (t_float *)(w[4]);

    t_float *out = (t_float *)(w[5]);
    int n = (int)(w[6]);
    float in;
    float pt,pt1;
    
    float x1 = x->x_1;
    float x2 = x->x_2;
    float x3 = x->x_3;
    float x4 = x->x_4;
    float ys1 = x->y_1;
    float ys2 = x->y_2;
    float ys3 = x->y_3;
    float ys4 = x->y_4;


   while (n--) {
     if (*p > 8140) *p = 8140.;
     *k = calc_k(*p,*k);
     pt =*p;
     pt1=(pt+1)*0.76923077;
     in = *in1++ - *k*ys4;
     ys1 = (pt1)*in + 0.3*x1 - pt*ys1;
     x1 = in;
     ys2 = (pt1)*ys1 + 0.3*x2 - pt*ys2;
     x2 = ys1;
     ys3 = (pt1)*ys2 + 0.3 *x3 - pt*ys3;
     x3 = ys2;
     ys4 = (pt1)*ys3 + 0.3*x4 - pt*ys4;
     x4 = ys3;
     *out++ = ys4;
   }

   
    x->y_1 = ys1;
    x->y_2 = ys2;
    x->y_3 = ys3;
    x->y_4 = ys4;
    x->x_1 = x1;
    x->x_2 = x2;
    x->x_3 = x3;
    x->x_4 = x4;

    return (w+7);
}


#define CLIP(x)  x = ((x) > 1.0 ? (1.0) : (x))

t_int *moog_perf8(t_int *w)
{
    t_moog* x = (t_moog*) (w[1]); 
    t_float *in1 = (t_float *)(w[2]);
    t_float *p = (t_float *)(w[3]);
    t_float *k = (t_float *)(w[4]);
    t_float *out = (t_float *)(w[5]);
    int n = (int)(w[6]);

    t_float x1 = x->x_1;
    t_float x2 = x->x_2;
    t_float x3 = x->x_3;
    t_float x4 = x->x_4;
    t_float ys1 = x->y_1;
    t_float ys2 = x->y_2;
    t_float ys3 = x->y_3;
    t_float ys4 = x->y_4;
    t_float temp,temp2;
    t_float pt,pt1;
    t_float in;

    while (n--) {
      if (*p > 8140.) *p = 8140.;
      *k = calc_k(*p,*k);

     pt =*p* 0.01*0.0140845 - 0.9999999f;
     pt1=(pt+1.0)*0.76923077;
     in = *in1++ - *k*ys4;
     ys1 = pt1*(in + 0.3*x1) - pt*ys1;
     x1 = in;
     ys2 = pt1*(ys1 + 0.3*x2) - pt*ys2;
     x2 = ys1;
     ys3 = pt1*(ys2 + 0.3*x3) - pt*ys3;
     x3 = ys2;
     ys4 = pt1*(ys3 + 0.3*x4) - pt*ys4;
     x4 = ys3;
     *out++ = ys4;

      p++;k++;
    }

    x->y_1 = ys1;
    x->y_2 = ys2;
    x->y_3 = ys3;
    x->y_4 = ys4;
    x->x_1 = x1;
    x->x_2 = x2;
    x->x_3 = x3;
    x->x_4 = x4;
    
    return (w+7);
}

void dsp_add_moog(t_moog *x, t_sample *in1, t_sample *in2, t_sample *in3, t_sample *out, int n)
{
    if (n&7)
    	dsp_add(moog_perform, 6,(t_int)x, in1,in2,in3, out, n);
    else	
    	dsp_add(moog_perf8, 6,(t_int) x, in1, in2, in3, out, n);
}

static void moog_dsp(t_moog *x, t_signal **sp)
{
    dsp_add_moog(x,sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[3]->s_vec,sp[0]->s_n);
}


void moog_tilde_setup(void)
{
    moog_class = class_new(gensym("moog~"), (t_newmethod)moog_new, 0,
    	sizeof(t_moog), 0, A_GIMME, 0);
    class_addmethod(moog_class, nullfn, gensym("signal"), 0);
    class_addmethod(moog_class, (t_method)moog_reset, gensym("reset"), 0);
    class_addmethod(moog_class, (t_method)moog_dsp, gensym("dsp"), A_NULL);
}
