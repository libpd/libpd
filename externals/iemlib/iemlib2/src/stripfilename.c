/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include <string.h>

/* -------------------------- stripfilename ----------------------- */
/* -- strips the first n or last n characters, depending on sign -- */
/* ------- of the initial argument (set message argument) --------- */

static t_class *stripfilename_class;

typedef struct _stripfilename
{
  t_object x_obj;
  int      x_nr_char;
  char     x_mem[MAXPDSTRING];
} t_stripfilename;

static void stripfilename_symbol(t_stripfilename *x, t_symbol *s)
{
  if(x->x_nr_char < 0)/* cuts the string from the back */
  {
    int len = strlen(s->s_name);
    int i=len + x->x_nr_char;

    if(len > (MAXPDSTRING - 2))
      strncpy(x->x_mem, s->s_name, MAXPDSTRING - 2 - len);
    else
      strcpy(x->x_mem, s->s_name);
    if(i < 0)
      i = 0;
    x->x_mem[i] = 0;
    outlet_symbol(x->x_obj.ob_outlet, gensym(x->x_mem));
  }
  else if(x->x_nr_char > 0)/* starts the string at this new offset */
  {
    int len = strlen(s->s_name);
    int i=x->x_nr_char;

    if(len > (MAXPDSTRING - 2))
      strncpy(x->x_mem, s->s_name, MAXPDSTRING - 2 - len);
    else
      strcpy(x->x_mem, s->s_name);
    if(i > len)
      i = len;
    outlet_symbol(x->x_obj.ob_outlet, gensym(x->x_mem+i));
  }
  else
    outlet_symbol(x->x_obj.ob_outlet, s);
}

static void stripfilename_set(t_stripfilename *x, t_floatarg nr_char)
{
  x->x_nr_char = (int)nr_char;
}

static void *stripfilename_new(t_floatarg nr_char)
{
  t_stripfilename *x = (t_stripfilename *)pd_new(stripfilename_class);
  
  stripfilename_set(x, nr_char);
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void stripfilename_setup(void)
{
  stripfilename_class = class_new(gensym("stripfilename"), (t_newmethod)stripfilename_new,
    0, sizeof(t_stripfilename), 0, A_DEFFLOAT, 0);
  class_addsymbol(stripfilename_class, stripfilename_symbol);
  class_addmethod(stripfilename_class, (t_method)stripfilename_set, gensym("set"), A_FLOAT, 0);
//  class_sethelpsymbol(stripfilename_class, gensym("iemhelp/help-stripfilename"));
}
