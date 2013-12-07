/* 
 * atoi: ascii to integer converter
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
#include <stdlib.h>

static t_class *atoi_class;

typedef struct _atoi
{
  t_object x_obj;
  int i;
} t_atoi;
static void atoi_bang(t_atoi *x)
{
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_float(t_atoi *x, t_floatarg f)
{
  x->i = f;
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_symbol(t_atoi *x, t_symbol *s)
{
  int base=10;
  const char* c = s->s_name;
  if(c[0]=='0'){
    base=8;
    if (c[1]=='x')base=16;
  }
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}
static void atoi_list(t_atoi *x, t_symbol *s, int argc, t_atom *argv)
{
  int base=10;
  const char* c;
  ZEXY_USEVAR(s);

  if (argv->a_type==A_FLOAT){
    x->i=atom_getfloat(argv);
    outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
    return;
  }

  if (argc>1){
    base=atom_getfloat(argv+1);
    if (base<2) {
      error("atoi: setting base to 10");
      base=10;
    }
  }
  c=atom_getsymbol(argv)->s_name;
  x->i=strtol(c, 0, base);
  outlet_float(x->x_obj.ob_outlet, (t_float)x->i);
}

static void *atoi_new(void)
{
  t_atoi *x = (t_atoi *)pd_new(atoi_class);
  outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

void atoi_setup(void)
{
  atoi_class = class_new(gensym("atoi"), (t_newmethod)atoi_new, 0,
			 sizeof(t_atoi), 0, A_DEFFLOAT, 0);

  class_addbang(atoi_class, (t_method)atoi_bang);
  class_addfloat(atoi_class, (t_method)atoi_float);
  class_addlist(atoi_class, (t_method)atoi_list);
  class_addsymbol(atoi_class, (t_method)atoi_symbol);
  class_addanything(atoi_class, (t_method)atoi_symbol);

  zexy_register("atoi");
}
