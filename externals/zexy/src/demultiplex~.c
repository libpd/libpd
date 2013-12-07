/* 
 *  demux~ : demultiplex a signal to a specified output
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

static t_class *demux_class;

typedef struct _demux {
  t_object x_obj;

  int output;

  int n_out;
  t_sample **out;

} t_demux;

static void demux_output(t_demux *x, t_floatarg f)
{
  if ((f>=0)&&(f<x->n_out)){
    x->output=f;
  } else
    error("demultiplex: %d is channel out of range (0..%d)", (int)f, x->n_out);
}


static t_int *demux_perform(t_int *w)
{
  t_demux *x = (t_demux *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  int N = (int)(w[3]);
  int n = N;

  int channel=x->n_out;

  while(channel--){
    t_sample*out=x->out[channel];
    n=N;
    if(x->output==channel){
      while(n--)*out++=*in++;
    } else
      while(n--)*out++=0.f;
  }
  return (w+4);
}

static void demux_dsp(t_demux *x, t_signal **sp)
{
  int n = x->n_out;
  t_sample **dummy=x->out;
  while(n--)*dummy++=sp[x->n_out-n]->s_vec;
  dsp_add(demux_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
}


static void demux_helper(void)
{
  post("\n%c demux~\t:: demultiplex a signal to one of various outlets", HEARTSYMBOL);
  post("<#out>\t : the outlet-number (counting from 0) to witch the inlet is routed"
       "'help'\t : view this");
  post("creation : \"demux~ [arg1 [arg2...]]\"\t: the number of arguments equals the number of outlets\n");
}

static void demux_free(t_demux *x)
{
  freebytes(x->out, x->n_out * sizeof(t_sample *));
}

static void *demux_new(t_symbol *s, int argc, t_atom *argv)
{
  t_demux *x = (t_demux *)pd_new(demux_class);
  int i;
  ZEXY_USEVAR(s);
  ZEXY_USEVAR(argv);

  if (!argc)argc=2;
  x->n_out=argc;
  x->output=0;

  while(argc--)outlet_new(&x->x_obj, gensym("signal"));

  x->out = (t_sample **)getbytes(x->n_out * sizeof(t_sample *));
  i=x->n_out;
  while(i--)x->out[i]=0;

  return (x);
}

void demultiplex_tilde_setup(void)
{
  demux_class = class_new(gensym("demultiplex~"), (t_newmethod)demux_new, (t_method)demux_free, sizeof(t_demux), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)demux_new, gensym("demux~"), A_GIMME, 0);

  class_addfloat(demux_class, demux_output);
  class_addmethod(demux_class, (t_method)demux_dsp, gensym("dsp"), 0);
  class_addmethod(demux_class, nullfn, gensym("signal"), 0);

  class_addmethod(demux_class, (t_method)demux_helper, gensym("help"), 0);

  zexy_register("demultiplex~");
}
void demux_tilde_setup(void)
{
  demultiplex_tilde_setup();
}



