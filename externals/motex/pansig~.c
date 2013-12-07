#include "m_pd.h"
#include <math.h>

static t_class *pansig_class;

/*  #define RADCONST 0.017453293 */
#define RADCONST 0.785398163
#define ROOT2DIV2 0.707106781

typedef struct _pansig
{
  t_object x_obj;
  float x_f;
  float pan;
  float left;
  float right;
} t_pansig;

static void *pansig_new(t_symbol *s, int argc, t_atom *argv)
{
  t_pansig *x = (t_pansig *)pd_new(pansig_class);
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
  outlet_new(&x->x_obj, gensym("signal"));
  outlet_new(&x->x_obj, gensym("signal"));
  //  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("pansigf"));

  x->x_f = 0;
  return (x);
}

static void pansig_getpan(t_floatarg signal, t_floatarg *left, t_floatarg *right)
{
  double tmp, result;
  double angle;
  signal = signal < -1 ? -1 : signal;
  signal = signal > 1 ? 1 : signal;
  angle = signal * RADCONST;
  *right  = ROOT2DIV2 * (cos(angle) + sin(angle));
  *left  = ROOT2DIV2 * (cos(angle) - sin(angle));

/*    if(signal < 0) */
/*      signal = 0; */
/*    if(signal > 0.999) */
/*      signal = 0.999; */
/*    tmp = (tan(1.5866 * signal - 0.785398) + 1) / 2; */
/*    result = sqrt(tmp); */
/*    if (result < 0) */
/*      *right = 0; */
/*    else if (result > 1) */
/*      *right = 1; */
/*    else */
/*      *right = result; */
/*    tmp = tmp * -1 + 1; */
/*    if(tmp < 0)  */
/*      tmp = 0; */
/*    result = sqrt(tmp); */
/*    if (result < 0) */
/*      *left = 0; */
/*    else if (result > 1) */
/*      *left = 1; */
/*    else */
/*      *left = result; */
}

static t_int *pansig_perform(t_int *w)
{
  float *in1 = (t_float *)(w[1]);
  float *in2 = (t_float *)(w[2]);
  float *out1 = (t_float *)(w[3]);
  float *out2 = (t_float *)(w[4]);
  int n = (int)(w[5]);
  t_pansig *x = (t_pansig *)(w[6]);
  float value, value2, left, right;
  while  (n--) 
    {
      value = *in1++;
      value2 = *in2++;
      pansig_getpan(value2, &left, &right);
/*        *out1++ = value * x->left; */
/*        *out2++ = value * x->right;  */
      *out1++ = value * left;
      *out2++ = value * right; 
/*        *out1++ = value; */
/*        *out2++ = value;  */
    }
  return (w+7);
}

static void pansig_dsp(t_pansig *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  float *in1 = sp[0]->s_vec;
  float *in2 = sp[1]->s_vec;
  float *out1 = sp[2]->s_vec;
  float *out2 = sp[3]->s_vec;

  dsp_add(pansig_perform, 6,
	  in1, in2, out1, out2, n, x);
}

void pansig_f(t_pansig *x, t_floatarg f)
{
  double tmp, result;
  if(f < 0)
    f = 0;
  if(f > 0.999)
    f = 0.999;
  tmp = (tan(1.5866 * f - 0.785398) + 1) / 2;
  result = sqrt(tmp);
  if (result < 0)
    x->right = 0;
  else if (result > 1)
    x->right = 1;
  else
    x->right = result;
  tmp = tmp * -1 + 1;
  if(tmp < 0) 
    tmp = 0;
  result = sqrt(tmp);
  if (result < 0)
    x->left = 0;
  else if (result > 1)
    x->left = 1;
  else
    x->left = result;
}

void pansig_tilde_setup(void)
{
  pansig_class = class_new(gensym("pansig~"), (t_newmethod)pansig_new, 0,
			sizeof(t_pansig), 0, A_GIMME, 0);
  class_addmethod(pansig_class, nullfn, gensym("signal"), 0);

  class_addmethod(pansig_class, (t_method)pansig_dsp, gensym("dsp"), 0);
  //  class_addmethod(pansig_class, (t_method)pansig_f, gensym("pansigf"), A_FLOAT, 0);  
}
