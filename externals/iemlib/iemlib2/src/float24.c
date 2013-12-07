/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

/* -------------------------- float24 ----------------------- */
/* ----------- float-object with 24 bit accuracy ------------ */
/* -- concaternate a list of float-arguments to one string -- */
/* ----------- and convert it to a float-number ------------- */

typedef struct _float24
{
  t_object  x_obj;
  t_float   x_arg;
} t_float24;

static t_class *float24_class;

static float float24_calc_sum(int argc, t_atom *argv)
{
  int i;
  char str[1000], buf[100];
  
  str[0] = 0;
  for(i=0; i<argc; i++)
  {
    if(IS_A_FLOAT(argv, i))
    {
      sprintf(buf, "%g", (float)atom_getfloatarg(i, argc, argv));
      strcat(str, buf);
    }
  }
  return(atof(str));
}

static void float24_bang(t_float24 *x)
{
  outlet_float(x->x_obj.ob_outlet, x->x_arg);
}

static void float24_float(t_float24 *x, t_float f)
{
  x->x_arg = f;
  float24_bang(x);
}

static void float24_list(t_float24 *x, t_symbol *s, int argc, t_atom *argv)
{
  if(argc > 0)
    x->x_arg = float24_calc_sum(argc, argv);
  float24_bang(x);
}

static void *float24_new(t_symbol *s, int argc, t_atom *argv)
{
  t_float24 *x = (t_float24 *)pd_new(float24_class);
  
  outlet_new(&x->x_obj, &s_float);
  x->x_arg = 0.0f;
  if(argc > 0)
    x->x_arg = float24_calc_sum(argc, argv);
  return (x);
}

void float24_setup(void)
{
  float24_class = class_new(gensym("float24"), (t_newmethod)float24_new, 0,
    sizeof(t_float24), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)float24_new, gensym("f24"), A_GIMME, 0);
  class_addbang(float24_class, (t_method)float24_bang);
  class_addfloat(float24_class, (t_method)float24_float);
  class_addlist(float24_class, (t_method)float24_list);
//  class_sethelpsymbol(float24_class, gensym("iemhelp/help-float24"));
}
