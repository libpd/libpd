/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ aspeedlim --------------------------- */
/* -- reduces the flow of anything-messages to one message per -- */
/* -------------------- time interval in ms --------------------- */

struct _aspeedlim_proxy;

static t_class *aspeedlim_class;
static t_class *aspeedlim_proxy_class;

typedef struct _aspeedlim
{
  t_object                 x_obj;
  struct _aspeedlim_proxy  *x_proxy_inlet;
  t_clock                  *x_clock;
  float                    x_delay;
  int                      x_output_is_locked;
  int                      x_there_was_n_event;
  int                      x_size;
  int                      x_ac;
  t_atom                   *x_at;
  t_symbol                 *x_selector_sym;
} t_aspeedlim;

typedef struct _aspeedlim_proxy
{
  t_object     p_obj;
  t_aspeedlim  *p_owner;
} t_aspeedlim_proxy;

static void aspeedlim_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void aspeedlim_proxy_stop(t_aspeedlim_proxy *p)
{
  t_aspeedlim *x = p->p_owner;
  
  x->x_output_is_locked = 0;
  x->x_there_was_n_event = 0;
  clock_unset(x->x_clock);
}

static void aspeedlim_proxy_delay(t_aspeedlim_proxy *p, t_floatarg delay)
{
  t_aspeedlim *x = p->p_owner;
  
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
}

static void aspeedlim_tick(t_aspeedlim *x)
{
  if(x->x_there_was_n_event)
  {
    x->x_output_is_locked = 1;
    x->x_there_was_n_event = 0;
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac, x->x_at);
    clock_delay(x->x_clock, x->x_delay);
  }
  else
  {
    x->x_output_is_locked = 0;
    x->x_there_was_n_event = 0;
  }
}

static void aspeedlim_anything(t_aspeedlim *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (10 + ac)*sizeof(t_atom));
    x->x_size = 10 + ac;
  }
  x->x_ac = ac;
  x->x_selector_sym = s;
  aspeedlim_atcopy(av, x->x_at, ac);
  if(!x->x_output_is_locked)
  {
    x->x_output_is_locked = 1;
    x->x_there_was_n_event = 0;
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac, x->x_at);
    clock_delay(x->x_clock, x->x_delay);
  }
  else
    x->x_there_was_n_event = 1;
}

static void aspeedlim_free(t_aspeedlim *x)
{
  clock_free(x->x_clock);
  if(x->x_at)
    freebytes(x->x_at, x->x_size * sizeof(t_atom));
  if(x->x_proxy_inlet)
    pd_free((t_pd *)x->x_proxy_inlet);
}

static void *aspeedlim_new(t_floatarg delay)
{
  t_aspeedlim *x = (t_aspeedlim *)pd_new(aspeedlim_class);
  t_aspeedlim_proxy *p = (t_aspeedlim_proxy *)pd_new(aspeedlim_proxy_class);
  
  x->x_proxy_inlet = p;
  p->p_owner = x;
  x->x_size = 10;
	x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
	x->x_ac = 0;
  x->x_selector_sym = &s_bang;
  if(delay < 0.0)
    delay = 0.0;
  x->x_delay = delay;
  x->x_output_is_locked = 0;
  x->x_there_was_n_event = 0;
  x->x_clock = clock_new(x, (t_method)aspeedlim_tick);
  inlet_new((t_object *)x, (t_pd *)p, 0, 0);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void aspeedlim_setup(void)
{
  aspeedlim_class = class_new(gensym("aspeedlim"), (t_newmethod)aspeedlim_new,
    (t_method)aspeedlim_free, sizeof(t_aspeedlim), 0, A_DEFFLOAT, 0);
  class_addanything(aspeedlim_class, aspeedlim_anything);
  
  aspeedlim_proxy_class = class_new(gensym("_aspeedlim_proxy"), 0, 0, sizeof(t_aspeedlim_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addmethod(aspeedlim_proxy_class, (t_method)aspeedlim_proxy_stop, gensym("stop"), 0);
  class_addfloat(aspeedlim_proxy_class, (t_method)aspeedlim_proxy_delay);
}
