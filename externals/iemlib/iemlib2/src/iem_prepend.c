/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* ----------------------- iem_prepend --------------------------- */
/* -- concatenates message from cold (right) inlet with message -- */
/* ------- from hot (left) inlet and outputs it;   initial ------- */
/* --- arguments (prependix) are eqal to message of cold inlet --- */

struct _iem_prepend_proxy;

static t_class *iem_prepend_class;
static t_class *iem_prepend_proxy_class;

typedef struct _iem_prepend
{
  t_object                   x_obj;
  struct _iem_prepend_proxy  *x_proxy_inlet;
  int                        x_size;
  int                        x_ac;
  t_atom                     *x_at;
  t_symbol                   *x_selector_sym;
} t_iem_prepend;

typedef struct _iem_prepend_proxy
{
  t_object       p_obj;
  t_iem_prepend  *p_owner;
} t_iem_prepend_proxy;

static void iem_prepend_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void iem_prepend_bang(t_iem_prepend *x)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac, x->x_at);
}

static void iem_prepend_float(t_iem_prepend *x, t_floatarg f)
{
  if(x->x_selector_sym == &s_bang)
    outlet_float(x->x_obj.ob_outlet, f);
  else
  {
    SETFLOAT(x->x_at+x->x_ac, f);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac+1, x->x_at);
  }
}

static void iem_prepend_symbol(t_iem_prepend *x, t_symbol *s)
{
  if(x->x_selector_sym == &s_bang)
    outlet_symbol(x->x_obj.ob_outlet, s);
  else
  {
    SETSYMBOL(x->x_at+x->x_ac, s);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac+1, x->x_at);
  }
}

static void iem_prepend_pointer(t_iem_prepend *x, t_gpointer *gp)
{
  if(x->x_selector_sym == &s_bang)
    outlet_pointer(x->x_obj.ob_outlet, gp);
  else
  {
    SETPOINTER(x->x_at+x->x_ac, gp);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac+1, x->x_at);
  }
}

static void iem_prepend_list(t_iem_prepend *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac+x->x_ac+1) >= x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+x->x_ac+11)*sizeof(t_atom));
    x->x_size = ac + x->x_ac + 11;
  }
  if(x->x_selector_sym == &s_bang)
    outlet_anything(x->x_obj.ob_outlet, &s_list, ac, av);
  else
  {
    iem_prepend_atcopy(av, x->x_at + x->x_ac, ac);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac+ac, x->x_at);
  }
}

static void iem_prepend_anything(t_iem_prepend *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac+x->x_ac+2) >= x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+x->x_ac+12)*sizeof(t_atom));
    x->x_size = ac + x->x_ac + 12;
  }
  if(x->x_selector_sym == &s_bang)
    outlet_anything(x->x_obj.ob_outlet, s, ac, av);
  else
  {
    SETSYMBOL(x->x_at + x->x_ac, s);
    iem_prepend_atcopy(av, x->x_at+x->x_ac+1, ac);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym, x->x_ac+ac+1, x->x_at);
  }
}

/* begin of proxy methods (anything inlets) */

static void iem_prepend_proxy_bang(t_iem_prepend_proxy *p)
{
  t_iem_prepend *x = p->p_owner;

  x->x_ac = 0;
  x->x_selector_sym = &s_bang;
}

static void iem_prepend_proxy_float(t_iem_prepend_proxy *p, t_floatarg f)
{
  t_iem_prepend *x = p->p_owner;

  x->x_ac = 1;
  SETFLOAT(x->x_at, f);
  x->x_selector_sym = &s_list;
}

static void iem_prepend_proxy_symbol(t_iem_prepend_proxy *p, t_symbol *s)
{
  t_iem_prepend *x = p->p_owner;

  x->x_ac = 1;
  SETSYMBOL(x->x_at, s);
  x->x_selector_sym = &s_list;
}

static void iem_prepend_proxy_pointer(t_iem_prepend_proxy *p, t_gpointer *gp)
{
  t_iem_prepend *x = p->p_owner;

  x->x_ac = 1;
  SETPOINTER(x->x_at, gp);
  x->x_selector_sym = &s_list;
}

static void iem_prepend_proxy_list(t_iem_prepend_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_prepend *x = p->p_owner;

  if((2*ac+10) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (2*ac+10)*sizeof(t_atom));
    x->x_size = 2*ac+10;
  }
  x->x_ac = ac;
  x->x_selector_sym = &s_list;
  iem_prepend_atcopy(av, x->x_at, ac);
}

static void iem_prepend_proxy_anything(t_iem_prepend_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_prepend *x = p->p_owner;

  if((2*ac+11) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (2*ac+11)*sizeof(t_atom));
    x->x_size = 2*ac + 11;
  }
  x->x_ac = ac;
  x->x_selector_sym = s;
  iem_prepend_atcopy(av, x->x_at, ac);
}

/* end of proxy methods (anything inlets) */

static void iem_prepend_free(t_iem_prepend *x)
{
  if(x->x_at)
    freebytes(x->x_at, x->x_size * sizeof(t_atom));
  if(x->x_proxy_inlet)
    pd_free((t_pd *)x->x_proxy_inlet);
}

static void *iem_prepend_new(t_symbol *s, int ac, t_atom *av)
{
  t_iem_prepend *x = (t_iem_prepend *)pd_new(iem_prepend_class);
  t_iem_prepend_proxy *p = (t_iem_prepend_proxy *)pd_new(iem_prepend_proxy_class);

  x->x_proxy_inlet = p;
  p->p_owner = x;

  x->x_size = 30;
  if(ac > 10)
    x->x_size = 2*ac + 10;
  x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
  if(!ac)
  {
    x->x_ac = 0;
    x->x_selector_sym = &s_bang;
  }
  else
  {
    if(IS_A_FLOAT(av, 0))
    {
      iem_prepend_proxy_list(p, &s_list, ac, av);
    }
    else if(IS_A_SYMBOL(av, 0))
    {
      iem_prepend_proxy_anything(p, atom_getsymbol(av), ac-1, av+1);
    }
  }
  inlet_new((t_object *)x, (t_pd *)p, 0, 0);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_prepend_setup(void)
{
  iem_prepend_class = class_new(gensym("iem_prepend"),
    (t_newmethod)iem_prepend_new, (t_method)iem_prepend_free,
    sizeof(t_iem_prepend), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)iem_prepend_new, gensym("pp"), A_GIMME, 0);
	class_addcreator((t_newmethod)iem_prepend_new, gensym("prepend"), A_GIMME, 0);

  class_addbang(iem_prepend_class, (t_method)iem_prepend_bang);
  class_addpointer(iem_prepend_class, iem_prepend_pointer);
  class_addfloat(iem_prepend_class, (t_method)iem_prepend_float);
  class_addsymbol(iem_prepend_class, iem_prepend_symbol);
  class_addlist(iem_prepend_class, iem_prepend_list);
  class_addanything(iem_prepend_class, iem_prepend_anything);
//  class_sethelpsymbol(iem_prepend_class, gensym("iemhelp/help-iem_prepend"));

  iem_prepend_proxy_class = class_new(gensym("_iem_prepend_proxy"),
    0, 0, sizeof(t_iem_prepend_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addbang(iem_prepend_proxy_class, (t_method)iem_prepend_proxy_bang);
  class_addpointer(iem_prepend_proxy_class, iem_prepend_proxy_pointer);
  class_addfloat(iem_prepend_proxy_class, (t_method)iem_prepend_proxy_float);
  class_addsymbol(iem_prepend_proxy_class, iem_prepend_proxy_symbol);
  class_addlist(iem_prepend_proxy_class, iem_prepend_proxy_list);
  class_addanything(iem_prepend_proxy_class, iem_prepend_proxy_anything);
}
