/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------ iem_receive ---------------------------- */
/* -------- like millers r, but with setable receive label --------- */

struct _iem_receive_proxy;

static t_class *iem_receive_class;
static t_class *iem_receive_proxy_class;

typedef struct _iem_receive
{
  t_object                   x_obj;
  struct _iem_receive_proxy  *x_proxy_receiver;
  t_symbol                   *x_receive_label_sym;
} t_iem_receive;

typedef struct _iem_receive_proxy
{
  t_object       p_obj;
  t_iem_receive  *p_owner;
} t_iem_receive_proxy;

static void iem_receive_clear(t_iem_receive *x, t_symbol *s, int ac, t_atom *av)
{
	t_iem_receive_proxy *p=x->x_proxy_receiver;

  if(x->x_receive_label_sym)
    pd_unbind(&p->p_obj.ob_pd, x->x_receive_label_sym);
  x->x_receive_label_sym = 0;
}

static void iem_receive_set(t_iem_receive *x, t_symbol *s, int ac, t_atom *av)
{
	t_iem_receive_proxy *p=x->x_proxy_receiver;

	if(ac > 0)
  {
    if(IS_A_SYMBOL(av,0))
		{
			if(x->x_receive_label_sym)
        pd_unbind(&p->p_obj.ob_pd, x->x_receive_label_sym);
      x->x_receive_label_sym = atom_getsymbol(av);
			pd_bind(&p->p_obj.ob_pd, x->x_receive_label_sym);
		}
    else if(IS_A_FLOAT(av,0))
    {
      char str[32];
      
			if(x->x_receive_label_sym)
        pd_unbind(&p->p_obj.ob_pd, x->x_receive_label_sym);
      sprintf(str, "%g", atom_getfloat(av));
      x->x_receive_label_sym = gensym(str);
			pd_bind(&p->p_obj.ob_pd, x->x_receive_label_sym);
    }
  }
}

/* begin of proxy methods (anything inlets) */

static void iem_receive_proxy_bang(t_iem_receive_proxy *p)
{
	t_iem_receive *x = p->p_owner;

  outlet_bang(x->x_obj.ob_outlet);
}

static void iem_receive_proxy_float(t_iem_receive_proxy *p, t_floatarg f)
{
	t_iem_receive *x = p->p_owner;

  outlet_float(x->x_obj.ob_outlet, f);
}

static void iem_receive_proxy_symbol(t_iem_receive_proxy *p, t_symbol *s)
{
	t_iem_receive *x = p->p_owner;

  outlet_symbol(x->x_obj.ob_outlet, s);
}

static void iem_receive_proxy_pointer(t_iem_receive_proxy *p, t_gpointer *gp)
{
	t_iem_receive *x = p->p_owner;

  outlet_pointer(x->x_obj.ob_outlet, gp);
}

static void iem_receive_proxy_list(t_iem_receive_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_iem_receive *x = p->p_owner;

  outlet_list(x->x_obj.ob_outlet, &s_list, argc, argv);
}

static void iem_receive_proxy_anything(t_iem_receive_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_iem_receive *x = p->p_owner;

  outlet_anything(x->x_obj.ob_outlet, s, argc, argv);
}

/* end of proxy methods (anything inlets) */

static void iem_receive_free(t_iem_receive *x)
{
	t_iem_receive_proxy *p=x->x_proxy_receiver;

  if(x->x_receive_label_sym)
    pd_unbind(&p->p_obj.ob_pd, x->x_receive_label_sym);
	if(x->x_proxy_receiver)
    pd_free((t_pd *)x->x_proxy_receiver);
}

static void *iem_receive_new(t_symbol *s, int ac, t_atom *av)
{
  t_iem_receive *x = (t_iem_receive *)pd_new(iem_receive_class);
	t_iem_receive_proxy *p = (t_iem_receive_proxy *)pd_new(iem_receive_proxy_class);

  x->x_proxy_receiver = p;
  p->p_owner = x;
  
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av,0))
    {
      x->x_receive_label_sym = atom_getsymbol(av);
      pd_bind(&p->p_obj.ob_pd, x->x_receive_label_sym);
    }
    else if(IS_A_FLOAT(av,0))
    {
      char str[100];
      
      sprintf(str, "%g", atom_getfloat(av));
      x->x_receive_label_sym = gensym(str);
      pd_bind(&p->p_obj.ob_pd, x->x_receive_label_sym);
    }
    else
      x->x_receive_label_sym = 0;
  }
  else
    x->x_receive_label_sym = 0;
  
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_receive_setup(void)
{
  iem_receive_class = class_new(gensym("iem_receive"), (t_newmethod)iem_receive_new, 
    (t_method)iem_receive_free, sizeof(t_iem_receive), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)iem_receive_new, gensym("iem_r"), A_GIMME, 0);
	class_addmethod(iem_receive_class, (t_method)iem_receive_clear, gensym("clear"), A_GIMME, 0);
  class_addmethod(iem_receive_class, (t_method)iem_receive_set, gensym("set"), A_GIMME, 0);
//  class_sethelpsymbol(iem_receive_class, gensym("iemhelp/help-iem_receive"));

	iem_receive_proxy_class = class_new(gensym("_iem_receive_proxy"),
    0, 0, sizeof(t_iem_receive_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addbang(iem_receive_proxy_class, iem_receive_proxy_bang);
  class_addfloat(iem_receive_proxy_class, iem_receive_proxy_float);
	class_addsymbol(iem_receive_proxy_class, iem_receive_proxy_symbol);
	class_addpointer(iem_receive_proxy_class, iem_receive_proxy_pointer);
  class_addlist(iem_receive_proxy_class, iem_receive_proxy_list);
  class_addanything(iem_receive_proxy_class, iem_receive_proxy_anything);
}
