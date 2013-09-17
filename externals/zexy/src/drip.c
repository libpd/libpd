/* 
 * drip: unfold a parallel data-structure (*pack*age) into a sequence
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

/* -------------------- drip ------------------------------ */

/*
unfold a parallel data-structure (*pack*age) into a sequence
like a medical drip
you can adjust the drop-speed in [ms]
*/


#include "zexy.h"

static t_class *drip_class;

typedef struct _drip
{
  t_object x_obj;

  t_atom *buffer, *current;
  int bufsize;

  t_clock *x_clock;
  t_float  deltime;

  int    flush;
} t_drip;


static void drip_makebuffer(t_drip *x, int n, t_atom *list)
{
  if (x->buffer) {
    freebytes(x->buffer, x->bufsize * sizeof(t_atom));
    x->buffer = 0;
    x->bufsize = 0;
  }

  x->buffer = copybytes(list, n * sizeof(t_atom));
  x->bufsize = n;
  x->current = x->buffer;
}

static void drip_bang(t_drip *x)
{ outlet_bang(x->x_obj.ob_outlet);}


static void drip_all(t_drip *x, int argc, t_atom *argv)
{
  while (argc--) {
    switch (argv->a_type) {
    case A_FLOAT:
	outlet_float(x->x_obj.ob_outlet, atom_getfloat(argv));
      break;
    case A_SYMBOL:
	outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(argv));
      break;
    case A_POINTER:
	outlet_pointer(x->x_obj.ob_outlet, argv->a_w.w_gpointer);
      break;
    default:
	outlet_bang(x->x_obj.ob_outlet);
    }
    argv++;
  }
}

static void drip_tick(t_drip *x)
{
  switch (x->current->a_type) {
  case A_FLOAT:
    outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->current));
    break;
  case A_SYMBOL:
    outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(x->current));
    break;
  case A_POINTER:
    outlet_pointer(x->x_obj.ob_outlet, x->current->a_w.w_gpointer);
    break;
  case A_NULL:
    outlet_bang(x->x_obj.ob_outlet);
  default:
    break;
  }
   
  if (x->current + 1 >= x->buffer + x->bufsize) { /* ok, we're done */
    clock_unset(x->x_clock);
    x->current = 0;
  } else { /* do it again */
    x->current++;
    clock_delay(x->x_clock, x->deltime);
  }
}

static void drip_list(t_drip *x, t_symbol *s, int argc, t_atom *argv)
{
  ZEXY_USEVAR(s);
  if (x->flush && x->current) { /* do we want to flush */
    drip_all(x, x->bufsize - (x->current - x->buffer), x->current);
  }

  if (x->deltime >= 0.f) { /* do we want to SCHEDULE ? */
    /* outlet the first element */
    switch (argv->a_type) {
    case (A_FLOAT):
      outlet_float(x->x_obj.ob_outlet, atom_getfloat(argv));
      break;
    case (A_SYMBOL):
      outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(argv));
      break;
    case (A_POINTER):
      outlet_pointer(x->x_obj.ob_outlet, argv->a_w.w_gpointer);
      break;
    default:
      outlet_bang(x->x_obj.ob_outlet);
    }
    /* create a buffer and copy the remaining list into it */
    drip_makebuffer(x, argc-1, argv+1);
    /* set the clock and start */
    clock_delay(x->x_clock, x->deltime);
  } else { /* UNSCHEDULED */
    drip_all(x, argc, argv);
  }
}

static void drip_anything(t_drip *x, t_symbol *s, int argc, t_atom *argv)
{
  if (x->flush && x->current) { /* do we want to flush */
    drip_all(x, x->bufsize - (x->current - x->buffer), x->current);
  }

  /* outlet the first element */
  outlet_symbol(x->x_obj.ob_outlet, s);

  if (x->deltime >= 0.f) { /* do we want to SCHEDULE ? */
    /* create a buffer and copy the remaining list into it */
    drip_makebuffer(x, argc, argv);
    /* set the clock and start */
    clock_delay(x->x_clock, x->deltime);
  } else { /* UNSCHEDULED */
    drip_all(x, argc, argv);
  }
}

static void drip_free(t_drip *x)
{
  clock_free(x->x_clock);

  if (x->buffer) {
    freebytes(x->buffer, x->bufsize * sizeof(t_atom));
    x->buffer = 0;
    x->bufsize = 0;
  }
}


static void *drip_new(t_symbol *s, int argc, t_atom *argv)
{
  t_drip *x = (t_drip *)pd_new(drip_class);
  ZEXY_USEVAR(s);

  if (argc>1) x->flush = 1;
  else x->flush = 0;

  if (argc) x->deltime = atom_getfloat(argv);
  else x->deltime = -1.f;
  if (x->deltime < 0.f) x->deltime = -1.0;

  x->x_clock = clock_new(x, (t_method)drip_tick);
  floatinlet_new(&x->x_obj, &x->deltime);

  outlet_new(&x->x_obj, 0);
  return (x);
}

void drip_setup(void)
{
  drip_class = class_new(gensym("drip"), (t_newmethod)drip_new, 
			      (t_method)drip_free, sizeof(t_drip), 0 ,A_GIMME, 0);

  class_addcreator((t_newmethod)drip_new, gensym("unfold"), A_GIMME, 0);
  /* for historical reasons */
  
  class_addbang    (drip_class, drip_bang);
  class_addlist    (drip_class, drip_list);
  class_addanything(drip_class, drip_anything);
  zexy_register("drip");
}
