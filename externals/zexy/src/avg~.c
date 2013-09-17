/* 
 * avg~: calculate the average of a signal block
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

#include "zexy.h"

/* ------------------------ average~ ----------------------------- */

/* tilde object to take absolute value. */

static t_class *avg_class;

typedef struct _avg
{
  t_object x_obj;

  t_float n_inv;
  int blocks;
} t_avg;


/* average :: arithmetic mean of one signal-vector */

static t_int *avg_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);

  t_avg *x = (t_avg *)w[2];
  int n = (int)(w[3]);

  t_sample buf = 0.;

  while (n--)
    {
      buf += *in++;
    }
  outlet_float(x->x_obj.ob_outlet, buf*x->n_inv);

  return (w+4);
}

static void avg_dsp(t_avg *x, t_signal **sp)
{
  x->n_inv=1./sp[0]->s_n;
  dsp_add(avg_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *avg_new(void)
{
  t_avg *x = (t_avg *)pd_new(avg_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static void avg_help(void)
{
  post("avg~\t:: outputs the arithmetic mean of each signal-vector");
}


void avg_tilde_setup(void)
{
  avg_class = class_new(gensym("avg~"), (t_newmethod)avg_new, 0,
                        sizeof(t_avg), 0, A_DEFFLOAT, 0);
  class_addmethod(avg_class, nullfn, gensym("signal"), 0);
  class_addmethod(avg_class, (t_method)avg_dsp, gensym("dsp"), 0);

  class_addmethod(avg_class, (t_method)avg_help, gensym("help"), 0);
  zexy_register("avg~");
}
