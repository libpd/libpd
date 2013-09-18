/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib1 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <math.h>


/* -------- v2db - a rms-value to techn. dB  converter. --------- */

static t_class *v2db_class;

t_float v2db(t_float f)
{
  return (f <= 0 ? -199.9 : 8.6858896381*log(f));
}

static void v2db_float(t_object *x, t_float f)
{
  outlet_float(x->ob_outlet, v2db(f));
}

static void *v2db_new(void)
{
  t_object *x = (t_object *)pd_new(v2db_class);
  outlet_new(x, &s_float);
  return (x);
}

void v2db_setup(void)
{
  v2db_class = class_new(gensym("v2db"), v2db_new, 0,
    sizeof(t_object), 0, 0);
  class_addfloat(v2db_class, (t_method)v2db_float);
//  class_sethelpsymbol(v2db_class, gensym("iemhelp/help-v2db"));
}
