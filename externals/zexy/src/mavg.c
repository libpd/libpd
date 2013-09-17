/* 
 * mavg :: moving average filter
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

static t_class *mavg_class;

typedef struct _mavg
{
  t_object x_obj;

  t_float n_inv;
  t_float avg;
  int size;
  t_float *buf, *wp;
} t_mavg;

static void mavg_resize(t_mavg *x, t_float f)
{
  int i;
  t_float *dumbuf;

  f = (int)f;
  if ((f<1) || (f == x->size)) return;

  freebytes(x->buf, sizeof(t_float)*x->size);
  x->n_inv = 1.0/f;
  x->size = f;
  x->buf = getbytes(sizeof(t_float)*x->size);

  dumbuf = x->wp = x->buf;
  i = x->size;
  while(i--) *dumbuf++ = x->avg;
}

static void mavg_set(t_mavg *x, t_symbol *s, int argc, t_atom *argv)
{
  int i = x->size;
  t_float *dummy = x->buf;
  t_float f=(argc)?atom_getfloat(argv):x->avg;
  ZEXY_USEVAR(s);

  while (i--) *dummy++=f;

  x->wp = x->buf;
}

static void mavg_float(t_mavg *x, t_float f)
{
  int i = x->size;
  t_float dummy = 0;
  t_float *dumb = x->buf;

  *x->wp++ = f;
  if (x->wp == x->buf + x->size) x->wp = x->buf;

  while (i--) dummy += *dumb++;

  x->avg = dummy*x->n_inv;

  outlet_float(x->x_obj.ob_outlet,x->avg);
}

static void *mavg_new(t_floatarg f)
{
  t_mavg *x = (t_mavg *)pd_new(mavg_class);
  int i = (f<1)?2:f;
  t_float *dumbuf;

  outlet_new(&x->x_obj, gensym("float"));
  inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym(""));

  x->buf = x->wp = (t_float *)getbytes(sizeof(t_float) * i);
  x->size = i;
  x->n_inv = 1.0f/(t_float)i;

  dumbuf = x->buf;
  while (i--) *dumbuf++=0;

  return (x);
}

static void mavg_help(void)
{
  post("mavg\t:: moving average filter");
}

void mavg_setup(void)
{
  mavg_class = class_new(gensym("mavg"), (t_newmethod)mavg_new, 0,
			 sizeof(t_mavg), 0, A_DEFFLOAT, 0);

  class_addfloat(mavg_class, (t_method)mavg_float);

  class_addmethod(mavg_class, (t_method)mavg_help, gensym("help"), 0);
  class_addmethod(mavg_class, (t_method)mavg_set, gensym("set"), A_GIMME, 0);
  class_addmethod(mavg_class, (t_method)mavg_resize, gensym(""), A_DEFFLOAT, 0);

  zexy_register("mavg");
}
