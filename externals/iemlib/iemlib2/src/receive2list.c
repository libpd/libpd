/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* -------------------------- receive2list ------------------------------ */
/* -- converts received message to a list with a prepended float index -- */

struct _receive2list_proxy;

static t_class *receive2list_class;
static t_class *receive2list_proxy_class;

typedef struct _receive2list
{
  t_object                    x_obj;
	int                         x_max;
  struct _receive2list_proxy  **x_proxy_receiver;
	int                         x_size;
  t_atom                      *x_at;
} t_receive2list;

typedef struct _receive2list_proxy
{
  t_object        p_obj;
  t_receive2list  *p_owner;
	t_symbol        *p_receive_label_sym;
	int             p_index;
} t_receive2list_proxy;

static void receive2list_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void receive2list_clear(t_receive2list *x, t_symbol *s, int ac, t_atom *av)
{
	t_receive2list_proxy *p;
	int i, max=x->x_max;

	for(i=0; i<max; i++)
	{
		p = x->x_proxy_receiver[i];
		if(p->p_receive_label_sym)
      pd_unbind(&p->p_obj.ob_pd, p->p_receive_label_sym);
		p->p_receive_label_sym = 0;
	}
}

static void receive2list_add(t_receive2list *x, t_symbol *s, int ac, t_atom *av)
{
	t_receive2list_proxy *p;
	int i;

	if((ac > 1)&&(IS_A_FLOAT(av,0)))
  {
		i = atom_getint(av);
		if((i >= 0)&&(i < x->x_max))
    {
		  p = x->x_proxy_receiver[i];
      if(IS_A_SYMBOL(av,1))
			{
        if(p->p_receive_label_sym)
          pd_unbind(&p->p_obj.ob_pd, p->p_receive_label_sym);
        p->p_receive_label_sym = atom_getsymbol(av+1);
			  pd_bind(&p->p_obj.ob_pd, p->p_receive_label_sym);
			}
      else if(IS_A_FLOAT(av,1))
			{
        char str[32];
      
			  if(p->p_receive_label_sym)
          pd_unbind(&p->p_obj.ob_pd, p->p_receive_label_sym);
        sprintf(str, "%g", atom_getfloat(av+1));
        p->p_receive_label_sym = gensym(str);
			  pd_bind(&p->p_obj.ob_pd, p->p_receive_label_sym);
			}
    }
  }
}

/* begin of proxy methods (anything inlets) */

static void receive2list_proxy_bang(t_receive2list_proxy *p)
{
	t_receive2list *x = p->p_owner;

	SETFLOAT(x->x_at, p->p_index);
	outlet_list(x->x_obj.ob_outlet, &s_list, 1, x->x_at);
}

static void receive2list_proxy_float(t_receive2list_proxy *p, t_floatarg f)
{
	t_receive2list *x = p->p_owner;

	SETFLOAT(x->x_at, p->p_index);
	SETFLOAT(x->x_at+1, f);
	outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
}

static void receive2list_proxy_symbol(t_receive2list_proxy *p, t_symbol *s)
{
	t_receive2list *x = p->p_owner;

  SETFLOAT(x->x_at, p->p_index);
	SETSYMBOL(x->x_at+1, s);
	outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
}

static void receive2list_proxy_pointer(t_receive2list_proxy *p, t_gpointer *gp)
{
	t_receive2list *x = p->p_owner;

  SETFLOAT(x->x_at, p->p_index);
	SETPOINTER(x->x_at+1, gp);
	outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
}

static void receive2list_proxy_list(t_receive2list_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_receive2list *x = p->p_owner;

	if((argc+1) >= x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (argc+11)*sizeof(t_atom));
    x->x_size = argc + 11;
  }
	SETFLOAT(x->x_at, p->p_index);
	receive2list_atcopy(argv, x->x_at+1, argc);
  outlet_list(x->x_obj.ob_outlet, &s_list, argc+1, x->x_at);
}

static void receive2list_proxy_anything(t_receive2list_proxy *p, t_symbol *s, int argc, t_atom *argv)
{
	t_receive2list *x = p->p_owner;

	if((argc+2) >= x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (argc+12)*sizeof(t_atom));
    x->x_size = argc + 12;
  }
	SETFLOAT(x->x_at, p->p_index);
	SETSYMBOL(x->x_at+1, s);
	receive2list_atcopy(argv, x->x_at+2, argc);
  outlet_list(x->x_obj.ob_outlet, &s_list, argc+2, x->x_at);
}

/* end of proxy methods (anything inlets) */

static void receive2list_free(t_receive2list *x)
{
	t_receive2list_proxy *p;
	int i, max = x->x_max;

	for(i=0; i<max; i++)
	{
		p = x->x_proxy_receiver[i];
		if(p->p_receive_label_sym)
      pd_unbind(&p->p_obj.ob_pd, p->p_receive_label_sym);
		if(x->x_proxy_receiver[i])
		  pd_free((t_pd *)x->x_proxy_receiver[i]);
	}
	if(x->x_proxy_receiver)
	  freebytes(x->x_proxy_receiver, x->x_max * sizeof(t_receive2list_proxy *));
	if(x->x_at)
	  freebytes(x->x_at, x->x_size * sizeof(t_atom));
}

static void *receive2list_new(t_floatarg fmax)
{
  t_receive2list *x = (t_receive2list *)pd_new(receive2list_class);
	t_receive2list_proxy *p;
	int i, max = (int)fmax;

	if(max <= 0)
    max = 80;
  x->x_max = max;
	x->x_proxy_receiver = (t_receive2list_proxy **)getbytes(x->x_max * sizeof(t_receive2list_proxy *));
	x->x_size = 12;
	x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
	for(i=0; i<max; i++)
	{
    x->x_proxy_receiver[i] = (t_receive2list_proxy *)pd_new(receive2list_proxy_class);
		p = x->x_proxy_receiver[i];
    p->p_owner = x;
		p->p_receive_label_sym = 0;
		p->p_index = i;
	}
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void receive2list_setup(void)
{
  receive2list_class = class_new(gensym("receive2list"), (t_newmethod)receive2list_new, 
    (t_method)receive2list_free, sizeof(t_receive2list), 0, A_DEFFLOAT, 0);
	class_addmethod(receive2list_class, (t_method)receive2list_clear, gensym("clear"), A_GIMME, 0);
  class_addmethod(receive2list_class, (t_method)receive2list_add, gensym("add"), A_GIMME, 0);
//  class_sethelpsymbol(receive2list_class, gensym("iemhelp/help-receive2list"));

	receive2list_proxy_class = class_new(gensym("_receive2list_proxy"),
    0, 0, sizeof(t_receive2list_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addbang(receive2list_proxy_class, receive2list_proxy_bang);
  class_addfloat(receive2list_proxy_class, receive2list_proxy_float);
	class_addsymbol(receive2list_proxy_class, receive2list_proxy_symbol);
	class_addpointer(receive2list_proxy_class, receive2list_proxy_pointer);
  class_addlist(receive2list_proxy_class, receive2list_proxy_list);
  class_addanything(receive2list_proxy_class, receive2list_proxy_anything);
}
