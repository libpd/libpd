/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ iem_alisttosym ---------------------------- */
/* ---------- converts a ASCII list of floats to a symbol ------------- */

static t_class *iem_alisttosym_class;

typedef struct _iem_alisttosym
{
  t_object x_obj;
  char     x_string[MAXPDSTRING];
} t_iem_alisttosym;

static void iem_alisttosym_list(t_iem_alisttosym *x, t_symbol *s, int ac, t_atom *av)
{
  t_int i=0, j=0, k=0;
  unsigned char uc=0;
  
  if(ac > 0)
  {
    for(i=0, j=0; i<ac; i++)
    {
      if(IS_A_FLOAT(av, i))
      {
        k = atom_getintarg(i, ac, av);
        if((k >= 0) && (k <= 255))
        {
          uc = (unsigned char)k;
          x->x_string[j++] = (char)uc;
        }
      }
      if(j >= (MAXPDSTRING - 2))
        break;
    }
  }
  x->x_string[j] = 0;
  outlet_symbol(x->x_obj.ob_outlet, gensym(x->x_string));
}

static void iem_alisttosym_free(t_iem_alisttosym *x)
{
}

static void *iem_alisttosym_new(void)
{
  t_iem_alisttosym *x = (t_iem_alisttosym *)pd_new(iem_alisttosym_class);
  
  x->x_string[0] = 0;
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void iem_alisttosym_setup(void)
{
  iem_alisttosym_class = class_new(gensym("iem_alisttosym"), (t_newmethod)iem_alisttosym_new,
    0, sizeof(t_iem_alisttosym), 0, 0);
  class_addlist(iem_alisttosym_class, iem_alisttosym_list);
}
