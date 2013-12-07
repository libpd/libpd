/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2008 */


#include "m_pd.h"
#include "iemlib.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>


/* ------------------------ iem_symtoalist --------------------- */
/* --------- converts a symbol to a ASCII list of floats ------- */

static t_class *iem_symtoalist_class;

typedef struct _iem_symtoalist
{
  t_object x_obj;
  char     x_string[MAXPDSTRING];
  t_atom   x_av[MAXPDSTRING];
} t_iem_symtoalist;

static void iem_symtoalist_symbol(t_iem_symtoalist *x, t_symbol *s)
{
  char *string=s->s_name;
  unsigned char uc;
  t_int i, n=strlen(string);
  
  for(i=0; i<n; i++)
  {
    uc = (unsigned char)string[i];
    SETFLOAT(x->x_av+i, (t_float)uc);
  }
  outlet_list(x->x_obj.ob_outlet, &s_list, n, x->x_av);
}

static void iem_symtoalist_float(t_iem_symtoalist *x, t_floatarg f)
{
  char string[40];
  unsigned char uc;
  t_int i, n;
  
  sprintf(string, "%g", f);
  n=strlen(string);
  for(i=0; i<n; i++)
  {
    uc = (unsigned char)string[i];
    SETFLOAT(x->x_av+i, (t_float)uc);
  }
  outlet_list(x->x_obj.ob_outlet, &s_list, n, x->x_av);
}

static void iem_symtoalist_free(t_iem_symtoalist *x)
{
}

static void *iem_symtoalist_new(void)
{
  t_iem_symtoalist *x = (t_iem_symtoalist *)pd_new(iem_symtoalist_class);
  
  x->x_string[0] = 0;
  SETFLOAT(x->x_av, 0.0);
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void iem_symtoalist_setup(void)
{
  iem_symtoalist_class = class_new(gensym("iem_symtoalist"), (t_newmethod)iem_symtoalist_new,
    0, sizeof(t_iem_symtoalist), 0, 0);
  class_addsymbol(iem_symtoalist_class, iem_symtoalist_symbol);
  class_addfloat(iem_symtoalist_class, iem_symtoalist_float);
}
