#include "m_pd.h"

/* ----------------------------- mandelbrot~ ----------------------------- */
t_class *mandelbrot_tilde_class;

typedef struct _mandelbrot_tilde
{
    t_object x_obj;
    float k;
    float cr;
    float ci;
} t_mandelbrot_tilde;

void *mandelbrot_tilde_new(t_symbol *s, t_floatarg f)
{
	t_mandelbrot_tilde *x = (t_mandelbrot_tilde *)pd_new(mandelbrot_tilde_class);
	inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
	floatinlet_new(&x->x_obj, &x->k);
	x->k = f;
	outlet_new(&x->x_obj, &s_signal);
	return (x);
}


static int mandelbrot_calc(t_float *cr, t_float *ci, int k)
{ 
   int i = -2;
   t_float zr = 0;
   t_float zi = 0;
   t_float z2r = 0;
   t_float z2i = 0;
   while ((z2r + z2i < 4 ) && (++i < k )) {
     z2r = zr * zr;
     z2i = zi * zi;
     zi = 2 * zr * zi + *ci;
     zr = z2r - z2i + *cr;
   }
   return(i);
}


static int mandelbrot_calc8(float cr, float ci, int k)
{ 
   int i = -2;
   t_float zr = 0;
   t_float zi = 0;
   t_float z2r = 0;
   t_float z2i = 0;
  while ((z2r + z2i < 4 ) && (++i < k )) {
     z2r = zr * zr;
     z2i = zi * zi;
     zi = 2 * zr * zi + ci;
     zr = z2r - z2i + cr;
  }
  return(i);
}


                                        
t_int *mandelbrot_tilde_perform(t_int *w)
{

    t_mandelbrot_tilde *x = (t_mandelbrot_tilde *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    t_float *out = (t_float *)(w[4]);
    int n = (int)(w[5]);
    while (n--) *out++ = (t_float)mandelbrot_calc (in1++,in2++, x->k);
    return (w+6);
}


t_int *mandelbrot_tilde_perf8(t_int *w)
{
  t_mandelbrot_tilde *x = (t_mandelbrot_tilde *)(w[1]);
  t_float *in1 = (t_float *)(w[2]);
  t_float *in2 = (t_float *)(w[3]);
  t_float *out = (t_float *)(w[4]);
  int n = (int)(w[5]);
  for (; n; n -= 8, in1 += 8, in2 += 8, out += 8)
  {
     float f0 = in1[0], f1 = in1[1], f2 = in1[2], f3 = in1[3];
     float f4 = in1[4], f5 = in1[5], f6 = in1[6], f7 = in1[7];

     float g0 = in2[0], g1 = in2[1], g2 = in2[2], g3 = in2[3];
     float g4 = in2[4], g5 = in2[5], g6 = in2[6], g7 = in2[7];
     out[0] = mandelbrot_calc8(f0 ,g0,x->k);
     out[1] = mandelbrot_calc8(f1 ,g1,x->k);
     out[2] = mandelbrot_calc8(f2 ,g2,x->k);
     out[3] = mandelbrot_calc8(f3 ,g3,x->k);
     out[4] = mandelbrot_calc8(f4 ,g4,x->k);
     out[5] = mandelbrot_calc8(f5 ,g5,x->k);
     out[6] = mandelbrot_calc8(f6 ,g6,x->k);
     out[7] = mandelbrot_calc8(f7 ,g7,x->k);

  }
  return (w+6);
}


void dsp_add_mandelbrot_tilde(t_mandelbrot_tilde *x,t_sample *in1, t_sample *in2, t_sample *out, int n)
{
    if (n&7)
    	dsp_add(mandelbrot_tilde_perform, 5, x, in1, in2, out, n);
    else	
    	dsp_add(mandelbrot_tilde_perf8, 5, x, in1, in2, out, n);
}

void mandelbrot_tilde_dsp(t_mandelbrot_tilde *x, t_signal **sp)
{
    dsp_add_mandelbrot_tilde(x,sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec, sp[0]->s_n);
}


void mandelbrot_tilde_setup(void)
{
    mandelbrot_tilde_class = class_new(gensym("mandelbrot~"), (t_newmethod)mandelbrot_tilde_new,
    0, sizeof(t_mandelbrot_tilde), 0, A_DEFFLOAT, 0);
    class_addmethod(mandelbrot_tilde_class, (t_method)mandelbrot_tilde_dsp, gensym("dsp"), 0);
    CLASS_MAINSIGNALIN(mandelbrot_tilde_class, t_mandelbrot_tilde, k);
}

