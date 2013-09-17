/* 
 * sigzero~: detect whether an entire signal vector is 0
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

/* ------------------------ sigzero~ ----------------------------- */
/*
  a very useful function, which detects, whether a signal is zeroes-only this block or not
  this is really great together with the "switch~"-object
*/

#include "zexy.h"


static t_class *sigzero_class;

typedef struct _sigzero
{
  t_object x_obj;
  int activate;
  int current; /* 0 == (signalblock == 0); 1==(signalblock != 0) */
} t_sigzero;

static void sigzero_activate(t_sigzero *x, t_floatarg activate)
{
  x->activate = (activate)?1:0;
}

static void sigzero_banged(t_sigzero *x)
{
  x->activate = 1;
}

static void sigzero_off(t_sigzero *x)
{
  x->activate = 0;
}

static t_int *sigzero_perform(t_int *w)
{
  t_sample *in = (t_sample *)w[1];
  t_sigzero *x = (t_sigzero *)w[2];
  int n = (int)w[3];

  int non_zero = 0;

  if (x->activate) {
    while (n--)
      {
        if (*in++ != 0.) {
          non_zero = 1;
          break;
        }
      }
    if (non_zero != x->current) {
      outlet_float(x->x_obj.ob_outlet, x->current = non_zero);
    }
  }

  return (w+4);
}

static void sigzero_dsp(t_sigzero *x, t_signal **sp)
{
  dsp_add(sigzero_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void sigzero_tilde_helper(void)
{
  post("\n%c sigzero~-object :: for detecting whether a signal is currently zero or not", HEARTSYMBOL);
  post("'bang'\t: turn the detector on\n"
       "'off'\t: turn it off\n"
       "<1/0>\t: turn it on/off\n"
       "'help'\t: view this\n"
       "signal~");
  post("outlet :: 1/0\t: signal turned to non-zero/zero\n");
}

static void *sigzero_new(void)
{
  t_sigzero *x = (t_sigzero *)pd_new(sigzero_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void sigzero_tilde_setup(void)
{
  sigzero_class = class_new(gensym("sigzero~"), (t_newmethod)sigzero_new, 0,
                            sizeof(t_sigzero), 0, 0);
  class_addfloat(sigzero_class, sigzero_activate);
  class_addbang(sigzero_class, sigzero_banged);
  class_addmethod(sigzero_class, (t_method)sigzero_off, gensym("off"), 0);

  class_addmethod(sigzero_class, nullfn, gensym("signal"), 0);
  class_addmethod(sigzero_class, (t_method)sigzero_dsp, gensym("dsp"), 0);

  class_addmethod(sigzero_class, (t_method)sigzero_tilde_helper, gensym("help"), 0);
  zexy_register("sigzero~");
}
