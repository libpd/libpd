/* 
 * dfreq~: cound zero-crossings as a simple fequency estimation
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


#include <stdio.h>

#include "zexy.h"

/* ------------------------ dspobj~ ----------------------------- */

/* datendefinition */

static t_class *dfreq_class;

typedef struct _dfreq
{
  t_object x_obj;

  t_sample freq;  /*frequenz variable */
  t_sample alt;
  t_sample sampcount;
  t_sample sr;
} t_dfreq;


static t_int *dfreq_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_sample *out = (t_sample *)(w[2]);
  int n = (int)(w[3]);
  t_dfreq *x = (t_dfreq *) w[4];

  t_sample a = x->alt;
  t_sample  c = x->sampcount;
  t_sample freq = x->freq;
  t_sample  sr=x->sr;

  t_sample delta_inv;

  while (n--) {

    if( (a * *in) < 0 && (a < *in)){

      /* interpolate for real zerocross */
      delta_inv = 1./(*in-a);
      if(c > 0.0)
        freq = sr / ((t_sample) c + a*delta_inv);
      else
        freq = sr;

      c = *in*delta_inv; /*rest of time */
    };

    a = *in;
    in++;
    c += 1.0;
    *out++ = freq;
  }

  x->alt = a;  
  x->sampcount = c;
  x->freq=freq;

  return (w+5);
}

static void dfreq_dsp(t_dfreq *x, t_signal **sp)
{
  dsp_add(dfreq_perform, 4, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n,x);
}



static void *dfreq_new(void)
{
  t_dfreq *x = (t_dfreq *)pd_new(dfreq_class);
  outlet_new(&x->x_obj, gensym("signal"));

  x->sr = sys_getsr();

  return (x);
}

static void dfreq_tilde_helper(void)
{
  post("\n%c dfreq~\t :: pitch-detector that counts zero-crossings", HEARTSYMBOL);
  post("\noutputs a frequency estimate as a stream~ that will be updated every zero-X");
  post("\ncreation::\t'dfreq~': that's all");
}


void dfreq_tilde_setup(void)
{
  dfreq_class = class_new(gensym("dfreq~"), (t_newmethod)dfreq_new, 0,
                          sizeof(t_dfreq), 0, A_NULL);
  class_addmethod(dfreq_class, nullfn, gensym("signal"), 0);
  class_addmethod(dfreq_class, (t_method)dfreq_dsp, gensym("dsp"), 0);

  class_addmethod(dfreq_class, (t_method)dfreq_tilde_helper, gensym("help"), 0);
  zexy_register("dfreq~");
}
