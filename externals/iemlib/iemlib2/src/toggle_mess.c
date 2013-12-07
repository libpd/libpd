/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ toggle_mess ---------------------------- */
/* -- inital arguments building a set of messages, any incomming --- */
/* -- increments the internal counter and outputs the n-th initial --*/
/* -- message. -- */

static t_class *toggle_mess_class;

typedef struct _toggle_mess
{
  t_object   x_obj;
  int        x_index;
  int        x_ac;
  t_atom     *x_at;
  void       *x_out_mid_sym;
  void       *x_out_rght_flt;
  t_symbol   *x_set;
} t_toggle_mess;

static void toggle_mess_bang(t_toggle_mess *x)
{
  int i = x->x_index;

  outlet_float(x->x_out_rght_flt, (t_float)i);
  if(IS_A_FLOAT(x->x_at, i))
    outlet_float(x->x_out_mid_sym, atom_getfloat(&x->x_at[i]));
  else
    outlet_anything(x->x_out_mid_sym, atom_getsymbol(&x->x_at[i]), 0, x->x_at);
  outlet_anything(x->x_obj.ob_outlet, x->x_set, 1, &x->x_at[i]);
}

static void toggle_mess_float(t_toggle_mess *x, t_floatarg f)
{
  x->x_index++;
  if(x->x_index >= x->x_ac)
    x->x_index = 0;
  toggle_mess_bang(x);
}

static void toggle_mess_set(t_toggle_mess *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 1) && (IS_A_FLOAT(av, 0)))
  {
    int x_index = (int)atom_getint(av);

    if(x_index < 0)
      x_index = 0;
    else if(x_index >= x->x_ac)
      x_index = x->x_ac;
    x->x_index = x_index;
    outlet_anything(x->x_obj.ob_outlet, x->x_set, 1, &x->x_at[x_index]);
  }
  else
    toggle_mess_float(x, 0);
}

static void toggle_mess_symbol(t_toggle_mess *x, t_symbol *s)
{
  toggle_mess_float(x, 0);
}

static void toggle_mess_anything(t_toggle_mess *x, t_symbol *s, int ac, t_atom *av)
{
  toggle_mess_float(x, 0);
}

static void toggle_mess_free(t_toggle_mess *x)
{
  freebytes(x->x_at, x->x_ac * sizeof(t_atom));
}

static void *toggle_mess_new(t_symbol *s, int ac, t_atom *av)
{
  t_toggle_mess *x = (t_toggle_mess *)pd_new(toggle_mess_class);
  int i;

  if(!ac)
  {
    post("toggle_mess-ERROR: must have at least one argument!");
    x->x_at = (t_atom *)0;
    return(0);
  }
  x->x_ac = ac;
  x->x_at = (t_atom *)getbytes(ac * sizeof(t_atom));
  for(i=0; i<ac; i++)
    x->x_at[i] = *av++;
  x->x_index = 0;
  x->x_set = gensym("set");
  outlet_new(&x->x_obj, &s_list);
  x->x_out_mid_sym = outlet_new(&x->x_obj, &s_list);
  x->x_out_rght_flt = outlet_new(&x->x_obj, &s_float);
  return(x);
}

void toggle_mess_setup(void)
{
  toggle_mess_class = class_new(gensym("toggle_mess"), (t_newmethod)toggle_mess_new,
    (t_method)toggle_mess_free, sizeof(t_toggle_mess), 0, A_GIMME, 0);
  class_addcreator((t_newmethod)toggle_mess_new, gensym("tm"), A_GIMME, 0);
  class_addbang(toggle_mess_class, (t_method)toggle_mess_bang);
  class_addmethod(toggle_mess_class, (t_method)toggle_mess_set, gensym("set"), A_GIMME, 0);
  class_addfloat(toggle_mess_class, (t_method)toggle_mess_float);
  class_addsymbol(toggle_mess_class, toggle_mess_symbol);
  class_addanything(toggle_mess_class, toggle_mess_anything);
//  class_sethelpsymbol(toggle_mess_class, gensym("iemhelp/help-toggle_mess"));
}
