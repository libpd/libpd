/* 
 * tavg~: arithmetic mean between last and current 'bang' (triggered average on signals)
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

static t_class *tavg_class;

typedef struct _tavg
{
  t_object x_obj;
  t_sample n_inv;
  t_sample buf;
  unsigned int blocks;
} t_tavgtilde;


static void tavg_bang(t_tavgtilde *x)
{
  if (x->blocks) {
    t_float result=x->buf*x->n_inv/x->blocks;
    outlet_float(x->x_obj.ob_outlet, result);
    x->blocks = 0;
    x->buf = 0.;
  }
}

static t_int *tavg_perform(t_int *w)
{
  t_sample *in = (t_sample *)(w[1]);
  t_tavgtilde *x = (t_tavgtilde *)w[2];
  int n = (int)(w[3]);
  t_sample buf = x->buf;

  while (n--) buf += *in++;
  x->buf = buf;
  x->blocks++;
  return (w+4);
}

static void tavg_dsp(t_tavgtilde *x, t_signal **sp)
{
  x->n_inv=1./sp[0]->s_n;
  dsp_add(tavg_perform, 3, sp[0]->s_vec, x, sp[0]->s_n);
}

static void *tavg_new(void)
{
  t_tavgtilde *x = (t_tavgtilde *)pd_new(tavg_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static void tavg_help(void)
{
  post("tavg~\t\t:: outputs the arithmetic mean of a signal when triggered");
  post("<bang>\t\t:  triggers the output");
}

void tavg_tilde_setup(void)
{
  tavg_class = class_new(gensym("tavg~"), (t_newmethod)tavg_new, 0,
                         sizeof(t_tavgtilde), 0, A_DEFFLOAT, 0);
  class_addmethod(tavg_class, nullfn, gensym("signal"), 0);
  class_addmethod(tavg_class, (t_method)tavg_dsp, gensym("dsp"), 0);

  class_addbang(tavg_class, tavg_bang);

  class_addmethod(tavg_class, (t_method)tavg_help, gensym("help"), 0);
  zexy_register("tavg~");
}
