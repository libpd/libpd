/* 
 * repeat: repeat a message multiple times
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

/* ------------------------- repeat ------------------------------- */

static t_class *repeat_class;

typedef struct _repeat
{
  t_object x_obj;
  t_float fcount;
} t_repeat;

static void repeat_anything(t_repeat *x, t_symbol *s, int argc, t_atom *argv)
{ 
  int i;
  i=x->fcount;
  if (i<0)i=1;
  while(i--)outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

static void *repeat_new(t_symbol*s, int argc, t_atom*argv)
{
  t_repeat *x = (t_repeat *)pd_new(repeat_class);
  ZEXY_USEVAR(s);
  if(argc){
    if(A_FLOAT==argv->a_type)
      x->fcount = atom_getfloat(argv);
    else return 0;
  } else x->fcount=2;
  floatinlet_new(&x->x_obj, &x->fcount);
  outlet_new(&x->x_obj, 0);
  return (x);
}

void repeat_setup(void)
{
  repeat_class = class_new(gensym("repeat"), (t_newmethod)repeat_new, 
			   0, sizeof(t_repeat), 0, A_GIMME, 0);
  class_addanything(repeat_class, repeat_anything);

  zexy_register("repeat");
}
