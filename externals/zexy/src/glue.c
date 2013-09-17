/* 
 * glue: glue two lists together (use [list append] instead)
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
#include <string.h>

static t_class *glue_class;

typedef struct _zglue
{
  t_object x_obj;

  t_atom *ap2, *ap;
  t_int n1, n2, n;

  t_int changed;
} t_glue;

static void glue_lst2(t_glue *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  x->changed = 1;
  if (x->n2 != argc) {
    freebytes(x->ap2, x->n2 * sizeof(t_atom));
    x->n2 = argc;
    x->ap2 = copybytes(argv, argc * sizeof(t_atom));
  } else memcpy(x->ap2, argv, argc * sizeof(t_atom));
}

static void glue_lst(t_glue *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  if (x->n != x->n2+argc) {
    freebytes(x->ap, x->n * sizeof(t_atom));
    x->n1 = argc;
    x->n  = x->n1+x->n2;
    x->ap = (t_atom *)getbytes(sizeof(t_atom)*x->n);
    memcpy(x->ap+argc, x->ap2, x->n2*sizeof(t_atom));
  } else if ((x->n1 != argc)||x->changed)memcpy(x->ap+argc, x->ap2, x->n2*sizeof(t_atom));

  x->n1 = argc;
  memcpy(x->ap, argv, x->n1*sizeof(t_atom));

  x->changed=0;

  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->n, x->ap);
}

static void glue_bang(t_glue *x)
{
  if (x->changed) {
    if (x->n1+x->n2 != x->n){
      t_atom *ap = (t_atom*)getbytes(sizeof(t_atom)*(x->n1+x->n2));
      memcpy(ap, x->ap, x->n1*sizeof(t_atom));
      freebytes(x->ap, sizeof(t_atom)*x->n);
      x->ap=ap;
      x->n=x->n1+x->n2;
    }
    memcpy(x->ap+x->n1, x->ap2, x->n2*sizeof(t_atom));
    x->changed=0;
  }

  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->n, x->ap);
}

static void glue_free(t_glue *x)
{
  freebytes(x->ap,  sizeof(t_atom)*x->n);
  freebytes(x->ap2, sizeof(t_atom)*x->n2);
}

static void *glue_new(t_symbol *s, int argc, t_atom *argv)
{
  t_glue *x = (t_glue *)pd_new(glue_class);
  ZEXY_USEVAR(s);

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("list"), gensym(""));
  outlet_new(&x->x_obj, 0);
  x->n =x->n2  = 0;
  x->ap=x->ap2 = 0;
  x->changed   = 0;

  if (argc)glue_lst2(x, gensym("list"), argc, argv);

  return (x);
}

static void glue_help(t_glue*x)
{
  post("\n%c glue\t\t:: glue together 2 lists (like [list append])", HEARTSYMBOL);
}

void glue_setup(void)
{
  glue_class = class_new(gensym("glue"), (t_newmethod)glue_new, 
			 (t_method)glue_free, sizeof(t_glue), 0, A_GIMME, 0);
  class_addlist(glue_class, glue_lst);
  class_addmethod  (glue_class, (t_method)glue_lst2, gensym(""), A_GIMME, 0);
  class_addbang(glue_class, glue_bang);
  class_addmethod  (glue_class, (t_method)glue_help, gensym("help"), 0);

  zexy_register("glue");
}
