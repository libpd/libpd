/* 
 * pack: a type-agnostic version of [pack]
 *
 * (c) 2007-2011 forum::für::umläute
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
 * this version of [pack] does not care about types, so you can send a symbol to a float inlet and vice versa
 * you can also initialize it with symbols, e.g. [pack foo bar] will output [list foo bar( when banged
 *
 * for know this object is named [zexy/pack], as there might be some issues with compatibility with the original [pack]
 */

#include "zexy.h"
#include <stdio.h>


/* ------------------------- zexy/pack ------------------------------- */

static t_class *zpack_class;
static t_class *zpackproxy_class;

typedef struct _zpack
{
  t_object x_obj;
  struct _zpackproxy  **x_proxy;

  t_inlet **in;

  t_atom*x_argv;
  int    x_argc;
} t_zpack;


typedef struct _zpackproxy
{
  t_pd  p_pd;
  t_zpack    *p_master;
  int id;
} t_zpackproxy;


static void setatom(t_zpack *x, t_atom*from, int to) {
  x->x_argv[to].a_type=from->a_type;
  x->x_argv[to].a_w   =from->a_w;
}

static void zpack_bang(t_zpack*x) {
  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->x_argc, x->x_argv);
}


static void zpack_any(t_zpack*x, t_symbol *s, int argc, t_atom *argv) {
  int i=0;
  int count=x->x_argc;

  if(NULL!=s && x->x_argc>0) {
    t_atom a;
    SETSYMBOL(&a, s);
    setatom(x, &a, i++);
    count--;
  }

  if(count>argc)
    count=argc;

  while(count-->0) {
    setatom(x, argv++, i++);
  }
 
 zpack_bang(x);
}

static void zpack_list(t_zpack*x, t_symbol *s, int argc, t_atom *argv) {
  zpack_any(x, 0, argc, argv);
}


static void zpack_proxy_list(t_zpackproxy *y, t_symbol *s, int argc, t_atom *argv)
{
  /* until we have lists of lists, this only uses the 1st element */
  if(argc>0) /* this filters out 'bang', and allows 'list', 'float' and 'symbol' */
    setatom(y->p_master, argv, y->id);
}
static void zpack_proxy_any(t_zpackproxy *y, t_symbol *s, int argc, t_atom *argv)
{
  /* until we have lists of lists, this only uses the selector */
  t_atom a;
  SETSYMBOL(&a, s); 
  setatom(y->p_master, &a, y->id);
}

static void *zpack_new(t_symbol *s, int argc, t_atom *argv)
{
  t_zpack *x = (t_zpack *)pd_new(zpack_class);
  int n =0;

  x->x_argc = (argc < 1)?2:argc;


  if(argc<1) {
    x->x_argv=(t_atom*)getbytes(2*sizeof(t_atom));
    SETFLOAT(x->x_argv+0, 0.f);
    SETFLOAT(x->x_argv+1, 0.f);
  } else {
    int i=0;
    x->x_argv=(t_atom*)getbytes(x->x_argc*sizeof(t_atom));
    for(i=0; i<x->x_argc; i++)
      setatom(x, argv+i, i);
  }

  x->in = (t_inlet **)getbytes(x->x_argc * sizeof(t_inlet *));
  x->x_proxy = (t_zpackproxy**)getbytes(x->x_argc * sizeof(t_zpackproxy*));

  x->in[0]     =0;
  x->x_proxy[0]=0;

  for (n = 1; n<x->x_argc; n++) {
    x->x_proxy[n]=(t_zpackproxy*)pd_new(zpackproxy_class);
    x->x_proxy[n]->p_master = x;
    x->x_proxy[n]->id=n;
    x->in[n] = inlet_new ((t_object*)x, (t_pd*)x->x_proxy[n], 0,0);
  }

  outlet_new(&x->x_obj, 0);
  return (x);
}

static void zpack_free(t_zpack*x){
  const int count = x->x_argc;

  if(x->in && x->x_proxy){
    int n=0;
    for(n=0; n<count; n++){
      if(x->in[n]){
        inlet_free(x->in[n]);
      }
      x->in[n]=0;
      if(x->x_proxy[n]){
        t_zpackproxy *y=x->x_proxy[n];
        y->p_master=0;
        y->id=0;
        pd_free(&y->p_pd);        
      }
      x->x_proxy[n]=0;      
    }
    freebytes(x->in, x->x_argc * sizeof(t_inlet *));
    freebytes(x->x_proxy, x->x_argc * sizeof(t_zpackproxy*));
  }
}

void zpack_setup(void)
{
  zpack_class = class_new(gensym("zexy/pack"), (t_newmethod)zpack_new,
			(t_method)zpack_free, sizeof(t_zpack), 0, A_GIMME,  0);
#if 0
  /* oops Pd>=0.42 allows us to override built-ins
   * this is bad as long as the 2 objects are not compatible */
  class_addcreator((t_newmethod)zpack_new, gensym("pack"), A_GIMME, 0);
#endif
  class_addbang(zpack_class, zpack_bang);
  class_addlist(zpack_class, zpack_list);
  class_addanything(zpack_class, zpack_any);

  zpackproxy_class = class_new(gensym("zpack proxy"), 0, 0,
			    sizeof(t_zpackproxy),
			    CLASS_PD | CLASS_NOINLET, 0);
  class_addlist(zpackproxy_class, zpack_proxy_list);
  class_addanything(zpackproxy_class, zpack_proxy_any);

  zexy_register("pack");
}

void pack_setup(void)
{
  zpack_setup();
}

