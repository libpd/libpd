/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* -------------------- iem_send ------------------------------ */
/* ------- like millers s, but with setable send label -------- */

struct _iem_send_proxy;

static t_class *iem_send_class;
static t_class *iem_send_proxy_class;

typedef struct _iem_send
{
  t_object                x_obj;
  struct _iem_send_proxy  *x_proxy_inlet;
  t_symbol                *x_send_label_sym;
} t_iem_send;

typedef struct _iem_send_proxy
{
  t_object    p_obj;
  t_iem_send  *p_owner;
} t_iem_send_proxy;

static void iem_send_bang(t_iem_send *x)
{
	if(x->x_send_label_sym)
    if(x->x_send_label_sym->s_thing)
      pd_bang(x->x_send_label_sym->s_thing);
}

static void iem_send_float(t_iem_send *x, t_floatarg f)
{
	if(x->x_send_label_sym)
    if(x->x_send_label_sym->s_thing)
      pd_float(x->x_send_label_sym->s_thing, f);
}

static void iem_send_symbol(t_iem_send *x, t_symbol *s)
{
	if(x->x_send_label_sym)
    if(x->x_send_label_sym->s_thing)
      pd_symbol(x->x_send_label_sym->s_thing, s);
}

static void iem_send_pointer(t_iem_send *x, t_gpointer *gp)
{
	if(x->x_send_label_sym)
    if(x->x_send_label_sym->s_thing)
      pd_pointer(x->x_send_label_sym->s_thing, gp);
}

static void iem_send_list(t_iem_send *x, t_symbol *s, int argc, t_atom *argv)
{
	if(x->x_send_label_sym)
    if(x->x_send_label_sym->s_thing)
      pd_list(x->x_send_label_sym->s_thing, s, argc, argv);
}

static void iem_send_anything(t_iem_send *x, t_symbol *s, int argc, t_atom *argv)
{
	if(x->x_send_label_sym)
	  if(x->x_send_label_sym->s_thing)
      typedmess(x->x_send_label_sym->s_thing, s, argc, argv);
}

/* begin of proxy methods (anything inlets) */

static void iem_send_proxy_clear(t_iem_send_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_send *x = p->p_owner;

  x->x_send_label_sym = 0;
}

static void iem_send_proxy_set(t_iem_send_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_send *x = p->p_owner;

	if(ac > 0)
  {
    if(IS_A_SYMBOL(av,0))
      x->x_send_label_sym = atom_getsymbol(av);
    else if(IS_A_FLOAT(av,0))
    {
      char str[32];
      
      sprintf(str, "%g", atom_getfloat(av));
      x->x_send_label_sym = gensym(str);
    }
  }
}

/* end of proxy methods (anything inlets) */

static void iem_send_free(t_iem_send *x)
{
  if(x->x_proxy_inlet)
    pd_free((t_pd *)x->x_proxy_inlet);
}

static void *iem_send_new(t_symbol *s, int ac, t_atom *av)
{
  t_iem_send *x = (t_iem_send *)pd_new(iem_send_class);
	t_iem_send_proxy *p = (t_iem_send_proxy *)pd_new(iem_send_proxy_class);

  x->x_proxy_inlet = p;
  p->p_owner = x;
  
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av,0))
    {
      x->x_send_label_sym = atom_getsymbol(av);
    }
    else if(IS_A_FLOAT(av,0))
    {
      char str[32];
      
      sprintf(str, "%g", atom_getfloat(av));
      x->x_send_label_sym = gensym(str);
    }
    else
      x->x_send_label_sym = 0;
  }
  else
    x->x_send_label_sym = 0;
	inlet_new((t_object *)x, (t_pd *)p, 0, 0);
  return (x);
}

void iem_send_setup(void)
{
  iem_send_class = class_new(gensym("iem_send"), (t_newmethod)iem_send_new, (t_method)iem_send_free,
    sizeof(t_iem_send), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)iem_send_new, gensym("iem_s"), A_GIMME, 0);
  class_addbang(iem_send_class, iem_send_bang);
  class_addfloat(iem_send_class, iem_send_float);
  class_addsymbol(iem_send_class, iem_send_symbol);
  class_addpointer(iem_send_class, iem_send_pointer);
  class_addlist(iem_send_class, iem_send_list);
  class_addanything(iem_send_class, iem_send_anything);
//  class_sethelpsymbol(iem_send_class, gensym("iemhelp/help-iem_send"));

	iem_send_proxy_class = class_new(gensym("_iem_send_proxy"),
    0, 0, sizeof(t_iem_send_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addmethod(iem_send_proxy_class, (t_method)iem_send_proxy_clear, gensym("clear"), A_GIMME, 0);
  class_addmethod(iem_send_proxy_class, (t_method)iem_send_proxy_set, gensym("set"), A_GIMME, 0);
}
