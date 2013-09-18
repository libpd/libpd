/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ transf_fader ----------------------- */
/* -- this loopkup tabel objekt makes a 2-point interpolation -- */
/* --------- between the pairs of determinating points --------- */

typedef struct _transf_fader
{
  t_object  x_obj;
  int       x_size;
  int       x_message;
  t_float     *x_array;
} t_transf_fader;

static t_class *transf_fader_class;

static void transf_fader_pairs(t_transf_fader *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc >= 4)
  {
    int i, j, k, ac=argc/2;
    int n=1000;
    int i_prev=0;
    int first=1;
    int i_delta;
    t_float val_delta;
    t_float delta;
    t_float val_prev=0.0f;
    t_float val=0.0f;
    t_float *vec=x->x_array;
    t_float fad_in, fad_out;

    for(j=0; j<n; j++)
    {
      vec[j] = -123456.0f;
    }

    for(j=0; j<ac; j++)
    {
      fad_in = atom_getfloat(argv++);
      while(fad_in < 0.0f)
        fad_in = 0.0f;
      while(fad_in > 1000.0f)
        fad_in = 1000.0f;

      fad_out = atom_getfloat(argv++);
      while(fad_out < -123455.0f)
        fad_out = -123455.0f;

      i = (int)fad_in;
      vec[i] = fad_out;
    }

    for(j=0; j<n; j++)
    {
      if(vec[j] > -123456.0f)
      {
        if(first)
        {
          first = 0;
          i_prev = j;
          val_prev = vec[j];
        }
        else
        {
          i = j;
          val = vec[j];
          i_delta = i - i_prev;
          val_delta = val - val_prev;
          if(i_delta > 1)
          {
            delta = val_delta / (t_float)i_delta;
            for(k=i_prev+1; k<i; k++)
            {
              vec[k] = val_prev + delta*(t_float)(k - i_prev);
            }
          }
          i_prev = i;
          val_prev = val;
        }
      }
    }
  }
  else
  {
    if(x->x_message)
      post("transf_fader-ERROR: less than 2 pairs make no sense");
  }
}

static void transf_fader_float(t_transf_fader *x, t_floatarg fad_in)
{
  t_float fad_out;
  t_float *vec=x->x_array;
  t_float fract;
  int i;

  while(fad_in < 0.0f)
    fad_in = 0.0f;
  while(fad_in > 999.0f)
    fad_in = 999.0f;

  i = (int)fad_in;
  fract = fad_in - (t_float)i;
  fad_out = vec[i] + fract*(vec[i+1] - vec[i]);
  if(fad_out > -123455.0f)
    outlet_float(x->x_obj.ob_outlet, fad_out);
}

static void transf_fader_free(t_transf_fader *x)
{
  freebytes(x->x_array, x->x_size * sizeof(t_float));
}

static void *transf_fader_new(t_symbol *s, int argc, t_atom *argv)
{
  t_transf_fader *x = (t_transf_fader *)pd_new(transf_fader_class);
  int i, n;

  x->x_size = 1001;
  x->x_message = 0;
  x->x_array = (t_float *)getbytes(x->x_size * sizeof(t_float));
  n = x->x_size;
  for(i=0; i<n; i++)
    x->x_array[i] = -123456.0f;
  transf_fader_pairs(x, gensym("pairs"), argc, argv);
  x->x_message = 1;
  outlet_new(&x->x_obj, &s_float);
  return (x);
}

void transf_fader_setup(void)
{
  transf_fader_class = class_new(gensym("transf_fader"), (t_newmethod)transf_fader_new, (t_method)transf_fader_free,
           sizeof(t_transf_fader), 0, A_GIMME, 0);
  class_addfloat(transf_fader_class, (t_method)transf_fader_float);
  class_addmethod(transf_fader_class, (t_method)transf_fader_pairs, gensym("pairs"), A_GIMME, 0);
//  class_sethelpsymbol(transf_fader_class, gensym("iemhelp/help-transf_fader"));
}
