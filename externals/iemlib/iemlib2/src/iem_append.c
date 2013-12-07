/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ----------------------- iem_append -------------------------- */
/* -- concatenates message from hot (left) inlet with message -- */
/* ------ from cold (right) inlet and outputs it;  initial ----- */
/* -- arguments (appendix) are eqal to message of cold inlet --- */


struct _iem_append_proxy;

static t_class *iem_append_class;
static t_class *iem_append_proxy_class;

typedef struct _iem_append
{
  t_object                  x_obj;
	struct _iem_append_proxy  *x_proxy_inlet;
  int                       x_size12;
  int                       x_size2;
  int                       x_ac1;
  int                       x_ac2;
  t_atom                    *x_at12;
  t_atom                    *x_at2;
  t_symbol                  *x_selector_sym1;
  t_symbol                  *x_selector_sym2;
  t_atomtype                x_type1;
  t_atomtype                x_type2;
} t_iem_append;

typedef struct _iem_append_proxy
{
  t_object      p_obj;
  t_iem_append  *p_owner;
} t_iem_append_proxy;

static void iem_append_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void iem_append_merge(t_iem_append *x, int off)
{
  if((x->x_ac1+x->x_ac2+1) > x->x_size12)
  {
    x->x_at12 = (t_atom *)resizebytes(x->x_at12, x->x_size12*sizeof(t_atom), 2*(x->x_ac1+x->x_ac2+1)*sizeof(t_atom));
    x->x_size12 = 2*(x->x_ac1+x->x_ac2+1);
  }
  if(off)
    SETSYMBOL(x->x_at12 + x->x_ac1, x->x_selector_sym2);
  iem_append_atcopy(x->x_at2, x->x_at12 + x->x_ac1 + off, x->x_ac2);
}

static void iem_append_out(t_iem_append *x)
{
  int off=0;
  
  if(x->x_type1 == A_GIMME)
  {
    if(x->x_type2 == A_COMMA)
      off = 1;
    else
      off = 0;
    iem_append_merge(x, off);
    outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac1+x->x_ac2+off, x->x_at12);
  }
  else if(x->x_type1 == A_COMMA)
  {
    if(x->x_type2 == A_COMMA)
      off = 1;
    else
      off = 0;
    iem_append_merge(x, off);
    outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym1, x->x_ac1+x->x_ac2+off, x->x_at12);
  }
  else if(x->x_type1 == A_NULL)/*depends on 2.part*/
  {
    iem_append_merge(x, 0);
    if(x->x_type2 == A_GIMME)
      outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac2, x->x_at12);
    else if(x->x_type2 == A_COMMA)
      outlet_anything(x->x_obj.ob_outlet, x->x_selector_sym2, x->x_ac2, x->x_at12);
    else if(x->x_type2 == A_FLOAT)
      outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->x_at12));
    else if(x->x_type2 == A_SYMBOL)
      outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(x->x_at12));
    else if(x->x_type2 == A_NULL)
      outlet_bang(x->x_obj.ob_outlet);
    else if(x->x_type2 == A_POINTER)
      outlet_pointer(x->x_obj.ob_outlet, (t_gpointer *)x->x_at12->a_w.w_gpointer);
  }
  else
  {
    if(x->x_type2 == A_COMMA)
      off = 1;
    else
      off = 0;
    iem_append_merge(x, off);
    if(x->x_type2 == A_NULL)
    {
      if(x->x_type1 == A_FLOAT)
        outlet_float(x->x_obj.ob_outlet, atom_getfloat(x->x_at12));
      else if(x->x_type1 == A_SYMBOL)
        outlet_symbol(x->x_obj.ob_outlet, atom_getsymbol(x->x_at12));
      else if(x->x_type1 == A_POINTER)
        outlet_pointer(x->x_obj.ob_outlet, (t_gpointer *)x->x_at12->a_w.w_gpointer);
    }
    else
      outlet_list(x->x_obj.ob_outlet, &s_list, x->x_ac1+x->x_ac2+off, x->x_at12);
  }
}

static void iem_append_bang(t_iem_append *x)
{
  x->x_ac1 = 0;
  x->x_type1 = A_NULL;
  iem_append_out(x);
}

static void iem_append_float(t_iem_append *x, t_float f)
{
  x->x_ac1 = 1;
  x->x_type1 = A_FLOAT;
  SETFLOAT(x->x_at12, f);
  iem_append_out(x);
}

static void iem_append_symbol(t_iem_append *x, t_symbol *s)
{
  x->x_ac1 = 1;
  x->x_type1 = A_SYMBOL;
  SETSYMBOL(x->x_at12, s);
  iem_append_out(x);
}

static void iem_append_pointer(t_iem_append *x, t_gpointer *gp)
{
  x->x_ac1 = 1;
  x->x_type1 = A_POINTER;
  SETPOINTER(x->x_at12, gp);
  iem_append_out(x);
}

static void iem_append_list(t_iem_append *x, t_symbol *s, int ac, t_atom *av)
{
  if((x->x_size2+ac+1) > x->x_size12)
  {
    x->x_at12 = (t_atom *)resizebytes(x->x_at12, x->x_size12*sizeof(t_atom), (x->x_size2+ac+11)*sizeof(t_atom));
    x->x_size12 = x->x_size2+ac+11;
  }
  x->x_ac1 = ac;
  x->x_type1 = A_GIMME;
  iem_append_atcopy(av, x->x_at12, ac);
  x->x_selector_sym1 = &s_list;
  iem_append_out(x);
}

static void iem_append_anything(t_iem_append *x, t_symbol *s, int ac, t_atom *av)
{
  if((x->x_size2+ac+2) > x->x_size12)
  {
    x->x_at12 = (t_atom *)resizebytes(x->x_at12, x->x_size12*sizeof(t_atom), (x->x_size2+ac+12)*sizeof(t_atom));
    x->x_size12 = x->x_size2+ac+12;
  }
  x->x_ac1 = ac;
  x->x_type1 = A_COMMA;
  iem_append_atcopy(av, x->x_at12, ac);
  x->x_selector_sym1 = s;
  iem_append_out(x);
}

/* begin of proxy methods (anything inlets) */

static void iem_append_proxy_bang(t_iem_append_proxy *p)
{
  t_iem_append *x = p->p_owner;

	x->x_ac2 = 0;
  x->x_type2 = A_NULL;
  x->x_selector_sym2 = &s_list;
}

static void iem_append_proxy_float(t_iem_append_proxy *p, t_float f)
{
  t_iem_append *x = p->p_owner;

	x->x_ac2 = 1;
  x->x_type2 = A_FLOAT;
  SETFLOAT(x->x_at2, f);
  x->x_selector_sym2 = &s_list;
}

static void iem_append_proxy_symbol(t_iem_append_proxy *p, t_symbol *s)
{
  t_iem_append *x = p->p_owner;

  x->x_ac2 = 1;
  x->x_type2 = A_SYMBOL;
  SETSYMBOL(x->x_at2, s);
  x->x_selector_sym2 = &s_list;
}

static void iem_append_proxy_pointer(t_iem_append_proxy *p, t_gpointer *gp)
{
  t_iem_append *x = p->p_owner;

  x->x_ac2 = 1;
  x->x_type2 = A_POINTER;
  SETPOINTER(x->x_at2, gp);
  x->x_selector_sym2 = &s_list;
}

static void iem_append_proxy_list(t_iem_append_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_append *x = p->p_owner;

	if(ac > x->x_size2)
  {
    x->x_at2 = (t_atom *)resizebytes(x->x_at2, x->x_size2*sizeof(t_atom), (ac+10)*sizeof(t_atom));
    x->x_size2 = (ac+10);
  }
  x->x_ac2 = ac;
  x->x_type2 = A_GIMME;
  x->x_selector_sym2 = &s_list;
  iem_append_atcopy(av, x->x_at2, ac);
}

static void iem_append_proxy_anything(t_iem_append_proxy *p, t_symbol *s, int ac, t_atom *av)
{
  t_iem_append *x = p->p_owner;

  if((ac+1) > x->x_size2)
  {
    x->x_at2 = (t_atom *)resizebytes(x->x_at2, x->x_size2*sizeof(t_atom), (ac+11)*sizeof(t_atom));
    x->x_size2 = ac+11;
  }
  x->x_ac2 = ac;
  x->x_type2 = A_COMMA;
  x->x_selector_sym2 = s;
  iem_append_atcopy(av, x->x_at2, ac);
}

/* end of proxy methods (anything inlets) */

static void iem_append_free(t_iem_append *x)
{
  if(x->x_at12)
    freebytes(x->x_at12, x->x_size12 * sizeof(t_atom));
  if(x->x_at2)
    freebytes(x->x_at2, x->x_size2 * sizeof(t_atom));
	if(x->x_proxy_inlet)
    pd_free((t_pd *)x->x_proxy_inlet);
}

static void *iem_append_new(t_symbol *s, int ac, t_atom *av)
{
	t_iem_append *x = (t_iem_append *)pd_new(iem_append_class);
  t_iem_append_proxy *p = (t_iem_append_proxy *)pd_new(iem_append_proxy_class);

  x->x_proxy_inlet = p;
  p->p_owner = x;
  
  x->x_type1 = A_NULL;
  x->x_selector_sym1 = &s_list;
  x->x_size2 = 10;
  if(ac > 5)
    x->x_size2 = 2*ac;
  x->x_at2 = (t_atom *)getbytes(x->x_size2 * sizeof(t_atom));
  x->x_size12 = x->x_size2 + 10;
  x->x_at12 = (t_atom *)getbytes(x->x_size12 * sizeof(t_atom));
  x->x_ac1 = 0;

  if(ac <= 0)
  {
    x->x_type2 = A_NULL;
    x->x_ac2 = 0;
    x->x_selector_sym2 = &s_list;
  }
  else
  {
    if(IS_A_FLOAT(av, 0))
    {
      if(ac == 1)
        iem_append_proxy_float(p, atom_getfloat(av));
      else
        iem_append_proxy_list(p, &s_list, ac, av);
    }
    else if(IS_A_SYMBOL(av, 0))
    {
      t_symbol *xsym=atom_getsymbol(av);
      
      if(xsym == &s_symbol)
      {
        if(ac > 1)
          iem_append_proxy_symbol(p, atom_getsymbol(av+1));
        else
          iem_append_proxy_symbol(p, gensym(""));
      }
      else if(xsym == &s_float)
      {
        if(ac > 1)
        {
          if(IS_A_FLOAT(av, 1))
            iem_append_proxy_float(p, atom_getfloat(av+1));
          else
            iem_append_proxy_float(p, 0.0f);
        }
        else
          iem_append_proxy_float(p, 0.0f);
      }
      else if(xsym == &s_list)
      {
        iem_append_proxy_list(p, &s_list, ac-1, av+1);
      }
      else
      {
        iem_append_proxy_anything(p, xsym, ac-1, av+1);
      }
    }
  }
  inlet_new((t_object *)x, (t_pd *)p, 0, 0);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_append_setup(void)
{
  iem_append_class = class_new(gensym("iem_append"),
    (t_newmethod)iem_append_new, (t_method)iem_append_free,
    sizeof(t_iem_append), 0, A_GIMME, 0);
	class_addcreator((t_newmethod)iem_append_new, gensym("merge_any"), A_GIMME, 0);
  class_addbang(iem_append_class, (t_method)iem_append_bang);
  class_addpointer(iem_append_class, iem_append_pointer);
  class_addfloat(iem_append_class, (t_method)iem_append_float);
  class_addsymbol(iem_append_class, iem_append_symbol);
  class_addlist(iem_append_class, iem_append_list);
  class_addanything(iem_append_class, iem_append_anything);
//  class_sethelpsymbol(iem_append_class, gensym("iemhelp/help-iem_append"));

	iem_append_proxy_class = class_new(gensym("_iem_append_proxy"),
    0, 0, sizeof(t_iem_append_proxy), CLASS_PD | CLASS_NOINLET, 0);
  class_addbang(iem_append_proxy_class, (t_method)iem_append_proxy_bang);
  class_addpointer(iem_append_proxy_class, iem_append_proxy_pointer);
  class_addfloat(iem_append_proxy_class, (t_method)iem_append_proxy_float);
  class_addsymbol(iem_append_proxy_class, iem_append_proxy_symbol);
  class_addlist(iem_append_proxy_class, iem_append_proxy_list);
  class_addanything(iem_append_proxy_class, iem_append_proxy_anything);
}
