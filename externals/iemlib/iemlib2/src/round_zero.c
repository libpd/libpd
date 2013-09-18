/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ round_zero ---------------------------- */
/* ------ small float numbers of an anything message within ------- */
/* ---- plus minus initial argument will be truncated to zero ----- */

static t_class *round_zero_class;

typedef struct _round_zero
{
  t_object  x_obj;
  t_float   x_bound;
} t_round_zero;

static void round_zero_anything(t_round_zero *x, t_symbol *s, int ac, t_atom *av)
{
  int i;
  t_float f, bound=x->x_bound;
  
  for(i=0; i<ac; i++)
  {
    if(IS_A_FLOAT(av, i))
    {
      f = atom_getfloatarg(i, ac, av);
      if((f <= bound)&&(f >= -bound))
      {
        f = 0.0f;
        SETFLOAT(av+i, f);
      }
    }
  }
  outlet_anything(x->x_obj.ob_outlet, s, ac, av);
}

static void *round_zero_new(t_floatarg bound)
{
  t_round_zero *x = (t_round_zero *)pd_new(round_zero_class);
  
    if(bound < 0.0f)
      x->x_bound = -bound;
    else
      x->x_bound = bound;
    outlet_new(&x->x_obj, &s_list);
    return (x);
}

void round_zero_setup(void)
{
  round_zero_class = class_new(gensym("round_zero"), (t_newmethod)round_zero_new,
    0, sizeof(t_round_zero), 0, A_DEFFLOAT, 0);
  class_addanything(round_zero_class, round_zero_anything);
//  class_sethelpsymbol(round_zero_class, gensym("iemhelp/help-round_zero"));
}
