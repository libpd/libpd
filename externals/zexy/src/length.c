/* 
 * length :: get the length of a list (use [list length] instead)
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

static t_class *length_class;
typedef struct _length
{
  t_object x_obj;
} t_length;

static void length_list(t_length *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  ZEXY_USEVAR(argv);
  outlet_float(x->x_obj.ob_outlet, (t_float)argc);
}
static void length_any(t_length *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  ZEXY_USEVAR(argv);
  outlet_float(x->x_obj.ob_outlet, (t_float)argc+1);
}

static void *length_new(void)
{
  t_length *x = (t_length *)pd_new(length_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void length_setup(void)
{
  length_class = class_new(gensym("length"), (t_newmethod)length_new, 0,
			 sizeof(t_length), 0, A_DEFFLOAT, 0);

  class_addlist(length_class, (t_method)length_list);
  class_addanything(length_class, (t_method)length_any);

  zexy_register("length");
}
