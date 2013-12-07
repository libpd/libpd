/* 
 *  repack    : (re)pack floats/symbols/pointers to fixed-length packages
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

/* -------------------- repack ------------------------------ */

/*
(re)pack a sequence of (packages of) atoms into a package of given length

"bang" gives out the current package immediately
the second inlet lets you change the default package size
*/

static t_class *repack_class;

typedef struct _repack
{
  t_object x_obj;
  t_atom  *buffer;
  int      bufsize;

  int      outputsize;
  int      current;
} t_repack;


static void repack_set(t_repack *x, t_float f)
{
  /* set the new default size */
  int n = f;

  if (n > 0) {

    /* flush all the newsized packages that are in the buffer */
    t_atom *dumbuf = x->buffer;
    int     dumcur = x->current;

    while (n <= dumcur) {
      outlet_list(x->x_obj.ob_outlet, gensym("list"), n, dumbuf);
      dumcur -= n;
      dumbuf += n;
    }

    if (dumcur < 0) error("this should never happen :: dumcur = %d < 0", dumcur);
    else {
      memcpy(x->buffer, dumbuf, dumcur * sizeof(t_atom));
      x->current = dumcur;
    }
	 
    if (n > x->bufsize) {
      dumbuf = (t_atom *)getbytes(n * sizeof(t_atom));
      memcpy(dumbuf, x->buffer, x->current * sizeof(t_atom));
      freebytes(x->buffer, x->bufsize * sizeof(t_atom));
      x->buffer =  dumbuf;
      x->bufsize = n;
    }
    
    x->outputsize = n;
  }
}

static void repack_bang(t_repack *x)
{
  /* output the list as far as we are now */
  outlet_list(x->x_obj.ob_outlet, gensym("list"), x->current, x->buffer);
  x->current = 0;
}

static void repack_float(t_repack *x, t_float f)
{
  /* add a float-atom to the list */
  SETFLOAT(&x->buffer[x->current], f);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}

static void repack_symbol(t_repack *x, t_symbol *s)
{
  /* add a sym-atom to the list */
  SETSYMBOL(&x->buffer[x->current], s);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}
static void repack_pointer(t_repack *x, t_gpointer *p)
{
  /* add a pointer-atom to the list */
  SETPOINTER(&x->buffer[x->current], p);
  x->current++;
  if (x->current >= x->outputsize) repack_bang(x);
}
static void repack_list(t_repack *x, t_symbol *s, int argc, t_atom *argv)
{
  int remain = x->outputsize - x->current;
  t_atom *ap = argv;
  ZEXY_USEVAR(s);

  if (argc >= remain) {
    memcpy(x->buffer+x->current, ap, remain * sizeof(t_atom));
    ap   += remain;
    argc -= remain;
    outlet_list(x->x_obj.ob_outlet, gensym("list"), x->outputsize, x->buffer);
    x->current = 0;
  }

  while (argc >= x->outputsize) {
    outlet_list(x->x_obj.ob_outlet, gensym("list"), x->outputsize, ap);
    ap += x->outputsize;
    argc -= x->outputsize;
  }

  memcpy(x->buffer + x->current, ap, argc * sizeof(t_atom));
  x->current += argc;
}
static void repack_anything(t_repack *x, t_symbol *s, int argc, t_atom *argv)
{
  SETSYMBOL(&x->buffer[x->current], s);
  x->current++;

  if (x->current >= x->outputsize) {
    repack_bang(x);
  }
  repack_list(x, gensym("list"), argc, argv);
}

static void *repack_new(t_floatarg f)
{
  t_repack *x = (t_repack *)pd_new(repack_class);



  x->outputsize = x->bufsize = (f>0.f)?f:2 ;
  x->current = 0;


  x->buffer = (t_atom *)getbytes(x->bufsize * sizeof(t_atom));

  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));
  outlet_new(&x->x_obj, 0);

  return (x);
}

void repack_setup(void)
{
  repack_class = class_new(gensym("repack"), (t_newmethod)repack_new, 
			   0, sizeof(t_repack), 0, A_DEFFLOAT, 0);
  
  class_addbang    (repack_class, repack_bang);
  class_addfloat   (repack_class, repack_float);
  class_addsymbol  (repack_class, repack_symbol);
  class_addpointer (repack_class, repack_pointer);
  class_addlist    (repack_class, repack_list);
  class_addanything(repack_class, repack_anything);
  class_addmethod  (repack_class, (t_method)repack_set, gensym(""), A_DEFFLOAT, 0);

  zexy_register("repack");
}
