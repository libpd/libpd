/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ------------------------------- iem_anything ---------------------------------- */
/* -- storage object for any message (bang, float, symbol, list, selector-list) -- */
/* ---------- with a hot and a cold inlet (like object float or symbol) ---------- */
/* ----------- initial arguments are equal to a message of cold inlet ------------ */

struct _iem_anything_proxy;

static t_class *iem_anything_class;
static t_class *iem_anything_proxy_class;

typedef struct _iem_anything
{
  t_object                    x_obj;
	struct _iem_anything_proxy  *x_proxy_inlet;
  int                         x_size;
  int                         x_ac;
  t_atom                      *x_at;
  t_symbol                    *x_selector_sym;
} t_iem_anything;

typedef struct _iem_anything_proxy
{
  t_object        p_obj;
  t_iem_anything  *p_owner;
} t_iem_anything_proxy;

static void iem_anything_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void iem_anything_anything(t_iem_anything *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (10 + ac)*sizeof(t_atom));
    x->x_size = 10 + ac;
  }
  x->x_ac = ac;
  x->x_selector_sym = s;
  iem_anything_atcopy(av, x->x_at, ac);
  outlet_anything(x->x_obj.ob_outlet, s, ac, av);
}

static void iem_anything_bang(t_iem_anything *x)
{
  if((x->x_selector_sym == &s_bang) && !x->x_ac)
  {
		outlet_bang(x->x_obj.ob_outlet);
  }
	else
	{
		outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac, x->x_at);
	}
}

/* begin of proxy methods (anything inlets) */

static void iem_anything_proxy_anything(t_iem_anything_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_anything *x = p->p_owner;

	if(ac > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (10 + ac)*sizeof(t_atom));
    x->x_size = 10 + ac;
  }
  x->x_ac = ac;
  x->x_selector_sym = s;
  iem_anything_atcopy(av, x->x_at, ac);
}

/* end of proxy methods (anything inlets) */

static void iem_anything_free(t_iem_anything *x)
{
  if(x->x_at)
    freebytes(x->x_at, x->x_size * sizeof(t_atom));
  if(x->x_proxy_inlet)
    pd_free((t_pd *)x->x_proxy_inlet);
}

static void *iem_anything_new(t_symbol *s, int ac, t_atom *av)
{
  t_iem_anything *x = (t_iem_anything *)pd_new(iem_anything_class);
  t_iem_anything_proxy *p = (t_iem_anything_proxy *)pd_new(iem_anything_proxy_class);

  x->x_proxy_inlet = p;
  p->p_owner = x;

	x->x_size = 10 + ac;
	x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
	x->x_ac = ac;
  if(!ac)
  {
    x->x_selector_sym = &s_bang;
  }
	else if(IS_A_SYMBOL(av, 0))
  {
		x->x_selector_sym = atom_getsymbol(av);
		x->x_ac--;
    iem_anything_proxy_anything(p, x->x_selector_sym, x->x_ac, av+1);
	}
  else
	{
		x->x_selector_sym = &s_list;
		iem_anything_proxy_anything(p, x->x_selector_sym, x->x_ac, av);
	}
  inlet_new((t_object *)x, (t_pd *)p, 0, 0);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_anything_setup(void)
{
  iem_anything_class = class_new(gensym("iem_anything"),
    (t_newmethod)iem_anything_new, (t_method)iem_anything_free,
    sizeof(t_iem_anything), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)iem_anything_new, gensym("any"), A_GIMME, 0);

  class_addanything(iem_anything_class, iem_anything_anything);
	class_addbang(iem_anything_class, iem_anything_bang);
//  class_sethelpsymbol(iem_anything_class, gensym("iemhelp/help-iem_anything"));

  iem_anything_proxy_class = class_new(gensym("_iem_anything_proxy"),
    0, 0, sizeof(t_iem_anything_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addanything(iem_anything_proxy_class, iem_anything_proxy_anything);
}
