#include "m_pd.h"
#include <math.h>

static t_class *voicing_detector_tilde_class;

typedef struct _voicing_control
{
  t_float *c_input;
  t_float f_sum_abs;
  t_atom otemp[4096];
  t_int method;
} t_voicing_control;

typedef struct _voicing_detector_tilde
{
  t_object x_obj;
  t_float f_dummy, f_thresh, f_low, f_high;
  t_voicing_control x_ctl;
  t_outlet *voiced, *prob;
} t_voicing_detector_tilde;

static t_int *voicing_detector_tilde_perform(t_int *w)
{
  t_voicing_detector_tilde     *x =   (t_voicing_detector_tilde *)(w[1]);  
  t_voicing_control *ctl = (t_voicing_control *)(w[2]);  
  t_int                n =                 (int)(w[3]);
  t_float            *in = ctl->c_input;
  if (x->f_high < x->f_low)
    {
      float tmp = x->f_low;
      x->f_low = x->f_high;
      x->f_high = tmp;
    }
  t_float current, previous, next, temp0, temp1, avg, max, peak0;
  current = previous = next = temp0 = temp1 = avg = max = peak0 = 0;
  t_float min = 1000;
  t_float samplerate = sys_getsr();
  t_float start = samplerate / x->f_high;
  start = start > 1 ? start : 1;
  t_float end = samplerate / x->f_low;
  end = end < (n-1) ? end : n-1;
  t_float result, prob, diff, diff2;
  t_int i = 0;
  int j;
  int l = n;
  int maxp = 0;
  int maxi = 0;
  float temp[n];
  ctl->f_sum_abs = 0.0;
  for (i=0;i<l;i++)
    {
      ctl->f_sum_abs += fabs(in[i]); /* I have to calculate f_sum_abs for the whole block before I can calculate amdf */
      temp[i] = 0.0;
      SETFLOAT(&ctl->otemp[i], 0.0);
    }
  for (i=1;i<l;i++)
    {
      temp1 = 0;
      for (j=start;j<=end;j++) /* the Average Magnitude Difference Function */
	  {
	    temp[j] = i + j < l ? in[i+j] : 0.0;
	    temp0 = atom_getfloatarg(i, 4096, ctl->otemp);
	    temp1 += fabs(in[j] - temp[j]);
	  }
      temp1 += ((float)i / (float)l) * ctl->f_sum_abs;
      SETFLOAT(&ctl->otemp[i], temp1);
    }

  for (i=start+1;i<end;i++)
    {
      previous= atom_getfloatarg(i-1, 2048, ctl->otemp);
      current = atom_getfloatarg(i, 2048, ctl->otemp);
      next = atom_getfloatarg(i+1, 2048, ctl->otemp);
      max = current > max ? current : max;
      min = current < min ? current : min;
      avg += current;
    }
  avg = avg / (end-start);
  diff = avg - min;
  diff2 = max - min;
  result = ctl->method == 0 ? ((avg - min) > (x->f_thresh) ? 1 : 0) : ((max - min) > x->f_thresh ? 1 : 0);
  prob = diff2 / max;
  outlet_float(x->prob, prob);
  outlet_float(x->voiced, result);
  return(w+4);
}

void voicing_detector_tilde_bound(t_voicing_detector_tilde *x, t_floatarg f1, t_floatarg f2)
{
  x->f_low = f1;
  x->f_high = f2;
}

void voicing_detector_tilde_method(t_voicing_detector_tilde *x, t_floatarg f)
{
  x->x_ctl.method = f > 0 ? 1 : 0;
}

void *voicing_detector_tilde_dsp(t_voicing_detector_tilde *x, t_signal **sp)
{
  x->x_ctl.c_input = sp[0]->s_vec;
  dsp_add(voicing_detector_tilde_perform, 3, x, &x->x_ctl, sp[0]->s_n);
  return (void *)x;
}

void *voicing_detector_tilde_new(t_floatarg f)
{
  t_voicing_detector_tilde *x = (t_voicing_detector_tilde *)pd_new(voicing_detector_tilde_class);
  x->f_thresh = f < 0 ? f : 25;
  x->f_low = 60;
  x->f_high = 500;

  floatinlet_new (&x->x_obj, &x->f_thresh);  
  x->voiced = outlet_new(&x->x_obj, gensym("float"));
  x->prob = outlet_new(&x->x_obj, gensym("float"));
  return (void *)x;
}

void voicing_detector_tilde_setup(void)
{
  voicing_detector_tilde_class = class_new(gensym("voicing_detector~"), (t_newmethod)voicing_detector_tilde_new, 0, sizeof(t_voicing_detector_tilde), CLASS_DEFAULT, A_DEFFLOAT, 0);
  
  post("\n-->AMDF voicing detector v0.2");
  post("-->by Nicolas Chetry <okin@altern.org>");
  post("-->& Edward Kelly <morph_2016@yahoo.co.uk>");
  
  class_addmethod(voicing_detector_tilde_class, (t_method)voicing_detector_tilde_bound, gensym("bound"), A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(voicing_detector_tilde_class, (t_method)voicing_detector_tilde_method, gensym("method"), A_DEFFLOAT, 0);
  class_addmethod(voicing_detector_tilde_class, (t_method)voicing_detector_tilde_dsp, gensym("dsp"), 0);

  CLASS_MAINSIGNALIN(voicing_detector_tilde_class, t_voicing_detector_tilde, f_dummy);
}
