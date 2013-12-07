/* 
 * wrap: wrap floats between two limits
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

static t_class *wrap_class;

typedef struct _wrap {
  t_object  x_obj;
  t_float f_upper, f_lower;
} t_wrap;


static void wrap_float(t_wrap *x, t_float f)
{
  if (x->f_lower==x->f_upper)
    outlet_float(x->x_obj.ob_outlet, x->f_lower);
  else {
    t_float modulo = fmod((f-x->f_lower),(x->f_upper-x->f_lower));
    if (modulo<0)modulo+=(x->f_upper-x->f_lower);
    
    outlet_float(x->x_obj.ob_outlet, x->f_lower+modulo);
  }
}
static void wrap_set(t_wrap *x, t_symbol *s, int argc, t_atom *argv){
  t_float f1, f2;
  ZEXY_USEVAR(s);
  switch (argc){
  case 0:
    f1=0.0;
    f2=1.0;
    break;
  case 1:
    f1=0.0;
    f2 = atom_getfloat(argv);
    break;
  default:
    f1 = atom_getfloat(argv);
    f2 = atom_getfloat(argv+1);
  }
  x->f_lower=(f1<f2)?f1:f2;
  x->f_upper=(f1>f2)?f1:f2;
}

static void *wrap_new(t_symbol *s, int argc, t_atom*argv)
{
  t_wrap *x = (t_wrap *)pd_new(wrap_class);
  wrap_set(x, s, argc, argv);

  outlet_new(&x->x_obj, gensym("float"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("set"));

  return (x);
}

static void wrap_help(t_wrap*x)
{
  post("\n%c wrap\t\t:: wrap a float between to boundaries", HEARTSYMBOL);
}

void wrap_setup(void) {
  wrap_class = class_new(gensym("wrap"),
			  (t_newmethod)wrap_new,
			  0, sizeof(t_wrap),
			  CLASS_DEFAULT, A_GIMME, A_NULL);

  class_addfloat (wrap_class, wrap_float);
  class_addmethod(wrap_class, (t_method)wrap_set, gensym("set"), A_GIMME, 0);
  class_addmethod(wrap_class, (t_method)wrap_help, gensym("help"), A_NULL);
  zexy_register("wrap");
}
