/* 
 * minmax :: get minimum and maximum of a list
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

static t_class *minmax_class;

typedef struct _minmax
{
  t_object x_obj;
  t_float min;
  t_float max;

  t_outlet *mino, *maxo;
} t_minmax;

static void minmax_bang(t_minmax *x)
{
  outlet_float(x->maxo,x->max);
  outlet_float(x->mino,x->min);
}

static void minmax_list(t_minmax *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  if(argc){
    t_float min = atom_getfloat(argv++);
    t_float max=min;
    argc--;
    
    while(argc--){
      t_float f = atom_getfloat(argv++);
      if (f<min)min=f;
      else if (f>max)max=f;
    }
    
    x->min=min;
    x->max=max;
  }
  minmax_bang(x);
}

static void *minmax_new(void)
{
  t_minmax *x = (t_minmax *)pd_new(minmax_class);

  x->mino=outlet_new(&x->x_obj, gensym("float"));
  x->maxo=outlet_new(&x->x_obj, gensym("float"));

  x->min = x->max = 0;

  return (x);
}

static void minmax_help(void)
{
  post("minmax\t:: get minimum and maximum of a list of floats");
}

void minmax_setup(void)
{
  minmax_class = class_new(gensym("minmax"), (t_newmethod)minmax_new, 0,
			 sizeof(t_minmax), 0, A_DEFFLOAT, 0);

  class_addlist(minmax_class, (t_method)minmax_list);
  class_addbang(minmax_class, (t_method)minmax_bang);
  class_addmethod(minmax_class, (t_method)minmax_help, gensym("help"), 0);

  zexy_register("minmax");
}
