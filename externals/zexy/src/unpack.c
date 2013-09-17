/* 
 * unpack: a type-agnostic version of [unpack]
 *
 * (c) 2007-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
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

/*
 * this version of [unpack] does not care about types, so the arguments are merely there for documentation reasons
 * (and to determine the number of outlets)
 * e.g. sending [symbol foo( to [zexy/unpack 0] will output [symbol foo(
 *
 * for know this object is named [zexy/unpack], as there might be some issues with compatibility with the original [unpack]
 */


#include "zexy.h"

/* ------------------------- zunpack ------------------------------- */

/* like pack, but does no type-checking */

static t_class *zunpack_class;

typedef struct _zunpack
{
  t_object x_obj;
  t_outlet**x_out;
  t_int x_numouts;
} t_zunpack;

static void zunpack_any(t_zunpack *x, t_symbol *s, int argc, t_atom *argv)
{
  int offset=(s!=NULL)?1:0;
  int count=((argc+offset)<(x->x_numouts))?(argc+offset):x->x_numouts;

  while(count-->offset) {
    outlet_list(x->x_out[count], gensym("list"), 1, argv+count-offset);
  }

  if(s!=NULL) {
    outlet_symbol(x->x_out[0], s);
  }
}

static void zunpack_list(t_zunpack *x, t_symbol *s, int argc, t_atom *argv)
{
  zunpack_any(x, 0, argc, argv);
}

static void zunpack_bang(t_zunpack *x)
{
  outlet_bang(x->x_out[0]);
}

static void zunpack_free(t_zunpack *x)
{
  int i=0;
  for(i=0; i<x->x_numouts; i++) {
    outlet_free(x->x_out[i]);
  }
  freebytes(x->x_out, x->x_numouts*sizeof(t_outlet*));

  x->x_numouts=0;
  x->x_out=0;  
}

static void *zunpack_new(t_symbol*s, int argc, t_atom*argv)
{
  t_zunpack *x = (t_zunpack *)pd_new(zunpack_class);
  int count=(argc>0)?argc:2;
  int i=0;
  
  x->x_numouts=count;
  x->x_out=(t_outlet**)getbytes(count*sizeof(t_outlet*));

  for(i=0; i<count; i++) {
    x->x_out[i]  =outlet_new(&x->x_obj, 0);
  } 

  return (x);
}

void zunpack_setup(void)
{
  
  zunpack_class = class_new(gensym("zexy/unpack"), 
                            (t_newmethod)zunpack_new, (t_method)zunpack_free, sizeof(t_zunpack), 
                            0,  A_GIMME, 0);
#if 0
  /* oops Pd-0.42 allows us to override built-ins
   * this is bad as long as the 2 objects are not compatible */
  class_addcreator((t_newmethod)zunpack_new, gensym("unpack"), A_GIMME, 0);
#endif

  class_addbang(zunpack_class, zunpack_bang);
  class_addlist(zunpack_class, zunpack_list);
  class_addanything(zunpack_class, zunpack_any);

  zexy_register("unpack");
}

void unpack_setup(void)
{
  zunpack_setup();
}
