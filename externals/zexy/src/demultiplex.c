/* 
 * demux :  demultiplex the input to a specified output  
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
#include <stdio.h>

/* ------------------------- demux ------------------------------- */

/*
  a demultiplexer
*/

static t_class *demux_class;

typedef struct _demux
{
  t_object x_obj;

  int n_out;
  t_outlet **out, *selected;


} t_demux;

static void demux_select(t_demux *x, t_float f)
{
  int n = ( (f<0) || (f>x->n_out) ) ? 0 : f;
  x->selected = x->out[n];
}

static void demux_list(t_demux *x, t_symbol *s, int argc, t_atom *argv)
{
  switch (argc) {
  case 0:
    outlet_bang(x->selected);
    break;
  case 1:
    switch (argv->a_type) {
    case A_FLOAT:
      outlet_float(x->selected, atom_getfloat(argv));
      break;
    case A_SYMBOL:
      outlet_symbol(x->selected, atom_getsymbol(argv));
      break;
    case A_POINTER:
       outlet_pointer(x->selected, argv->a_w.w_gpointer);
       break;
    default:
      outlet_list(x->selected, s, argc, argv);
    }
    break;
  default:
    outlet_list(x->selected, s, argc, argv);
  }
}
static void demux_any(t_demux *x, t_symbol *s, int argc, t_atom *argv)
{
  outlet_anything(x->selected, s, argc, argv);
}

static void *demux_new(t_symbol *s, int argc, t_atom *argv)
{
  t_demux *x = (t_demux *)pd_new(demux_class);
  int n = (argc < 2)?2:argc;
  ZEXY_USEVAR(s);
  ZEXY_USEVAR(argv);

  x->n_out = n - 1;
  
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("select"));
  x->out = (t_outlet **)getbytes(n * sizeof(t_outlet *));

  for (n=0; n<=x->n_out; n++) {
    x->out[n] = outlet_new(&x->x_obj, 0);
  }

  x->selected = x->out[0];

  return (x);
}

void demultiplex_setup(void)
{
  demux_class = class_new(gensym("demultiplex"), (t_newmethod)demux_new,
			      0, sizeof(t_demux), 0, A_GIMME,  0);
  class_addcreator((t_newmethod)demux_new, gensym("demux"), A_GIMME, 0);
  
  class_addanything (demux_class, demux_any);
  class_addlist     (demux_class, demux_list);

  class_addmethod   (demux_class, (t_method)demux_select, gensym("select"), A_DEFFLOAT, 0);

  zexy_register("demultiplex");
}
void demux_setup(void)
{
  demultiplex_setup();
}

