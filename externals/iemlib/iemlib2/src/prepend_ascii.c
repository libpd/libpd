/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* ------------------------- prepend_ascii ---------------------------- */
/* -- this object prepends to any incoming message a selector symbol -- */
/* --- containing the n-th character of the ASCII-table. "n" is the --- */
/* ----------------- first initial argument (integer) ----------------- */

static t_class *prepend_ascii_class;

typedef struct _prepend_ascii
{
  t_object    x_obj;
  t_atom      *x_at;
  int         x_size;
  int         x_ac;
  t_symbol    *x_sym;
} t_prepend_ascii;

static void prepend_ascii_atcopy(t_atom *src, t_atom *dst, int n)
{
  while(n--)
    *dst++ = *src++;
}

static void prepend_ascii_bang(t_prepend_ascii *x)
{
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac, x->x_at);
}

static void prepend_ascii_float(t_prepend_ascii *x, t_floatarg f)
{
  SETFLOAT(x->x_at+x->x_ac, f);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac+1, x->x_at);
}

static void prepend_ascii_symbol(t_prepend_ascii *x, t_symbol *s)
{
  SETSYMBOL(x->x_at+x->x_ac, s);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac+1, x->x_at);
}

static void prepend_ascii_pointer(t_prepend_ascii *x, t_gpointer *gp)
{
  SETPOINTER(x->x_at+x->x_ac, gp);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, x->x_ac+1, x->x_at);
}

static void prepend_ascii_list(t_prepend_ascii *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac+x->x_ac) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+x->x_ac)*sizeof(t_atom));
    x->x_size = (ac+x->x_ac);
  }
  prepend_ascii_atcopy(av, x->x_at+x->x_ac, ac);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, ac+x->x_ac, x->x_at);
}

static void prepend_ascii_anything(t_prepend_ascii *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac+x->x_ac+1) > x->x_size)
  {
    x->x_at = (t_atom *)resizebytes(x->x_at, x->x_size*sizeof(t_atom), (ac+x->x_ac+1)*sizeof(t_atom));
    x->x_size = (ac+x->x_ac+1);
  }
  SETSYMBOL(x->x_at+x->x_ac, s);
  prepend_ascii_atcopy(av, x->x_at+x->x_ac+1, ac);
  outlet_anything(x->x_obj.ob_outlet, x->x_sym, ac+x->x_ac+1, x->x_at);
}

static void prepend_ascii_free(t_prepend_ascii *x)
{
  if(x->x_at)
    freebytes(x->x_at, x->x_size * sizeof(t_atom));
}

static void *prepend_ascii_new(t_symbol *s, int ac, t_atom *av)
{
  if((ac <= 0) || (!IS_A_FLOAT(av,0)))
  {
    post("ERROR: prepend_ascii need a float between 1 and 255 as 1. arg. !!!");
    return(0);
  }
  else
  {
    t_prepend_ascii *x = (t_prepend_ascii *)pd_new(prepend_ascii_class);
    char str[2];
    int i;
    
    x->x_size = 10 + ac;
    x->x_at = (t_atom *)getbytes(x->x_size * sizeof(t_atom));
    str[0] = (char)((int)(atom_getfloatarg(0,ac,av))&0xff);
    str[1] = 0;
    x->x_sym = gensym(str);
    x->x_ac = ac - 1;
    for(i=1; i<ac; i++)
      x->x_at[i-1] = av[i];
    outlet_new(&x->x_obj, &s_list);
    return (x);
  }
}

void prepend_ascii_setup(void)
{
  prepend_ascii_class = class_new(gensym("prepend_ascii"), (t_newmethod)prepend_ascii_new,
    (t_method)prepend_ascii_free, sizeof(t_prepend_ascii), 0, A_GIMME, 0);
  class_addbang(prepend_ascii_class, (t_method)prepend_ascii_bang);
  class_addfloat(prepend_ascii_class, (t_method)prepend_ascii_float);
  class_addsymbol(prepend_ascii_class, prepend_ascii_symbol);
  class_addpointer(prepend_ascii_class, prepend_ascii_pointer);
  class_addlist(prepend_ascii_class, prepend_ascii_list);
  class_addanything(prepend_ascii_class, prepend_ascii_anything);
//  class_sethelpsymbol(prepend_ascii_class, gensym("iemhelp/help-prepend_ascii"));
}
