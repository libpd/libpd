/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <string.h>


/* ----------------------- splitfilename -------------------------- */
/* -- splits a symbol into 2 parts (path + file) at the position -- */
/* -- of the first separator-character beginnig from the right ---- */
/* ---------- eliminating the separator-character ----------------- */

static t_class *splitfilename_class;

typedef struct _splitfilename
{
  t_object x_obj;
  char     x_sep[2];
  char     x_mem[MAXPDSTRING];
  t_outlet *x_outpath;
  t_outlet *x_outfile;
} t_splitfilename;

static void splitfilename_separator(t_splitfilename *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac > 0)
  {
    if(IS_A_SYMBOL(av, 0))
    {
      char *name=av->a_w.w_symbol->s_name;

      if(strlen(name) == 1)
        x->x_sep[0] = name[0];
      else if(!strcmp(name, "backslash"))
        x->x_sep[0] = '\\';
      else if(!strcmp(name, "slash"))
        x->x_sep[0] = '/';
      else if(!strcmp(name, "blank"))
        x->x_sep[0] = ' ';
      else if(!strcmp(name, "space"))
        x->x_sep[0] = ' ';
      else if(!strcmp(name, "dollar"))
        x->x_sep[0] = '$';
      else if(!strcmp(name, "comma"))
        x->x_sep[0] = ',';
      else if(!strcmp(name, "semi"))
        x->x_sep[0] = ';';
      else if(!strcmp(name, "leftbrace"))
        x->x_sep[0] = '{';
      else if(!strcmp(name, "rightbrace"))
        x->x_sep[0] = '}';
      else
        x->x_sep[0] = '/';
    }
    else if(IS_A_FLOAT(av, 0))
    {
      t_int i=atom_getintarg(0, ac, av);

      x->x_sep[0] = (char)i + '0';/* you can set any separator-char by setting a number between -32 ... 223 */
    }
  }
  else
    x->x_sep[0] = 0;
}

static void splitfilename_symbol(t_splitfilename *x, t_symbol *s)
{
  t_int length = strlen(s->s_name);
  
  if(length)
  {
    if(x->x_sep[0])
    {
      char *sep_ptr=x->x_mem;

      if(length > (MAXPDSTRING - 2))
        strncpy(x->x_mem, s->s_name, MAXPDSTRING - 2 - length);
      else
        strcpy(x->x_mem, s->s_name);

      sep_ptr = strrchr(x->x_mem, x->x_sep[0]);/* points to the leftest separator-char-index of string */
      if((!sep_ptr) || ((sep_ptr - x->x_mem) < 0) || ((sep_ptr - x->x_mem) >= length))
      { /* JMZ: 20050701 : removed typecast (char*) to (int); this is not portable */
        outlet_symbol(x->x_outfile, &s_);
        outlet_symbol(x->x_outpath, gensym(x->x_mem));
      }
      else
      {
        *sep_ptr = 0;
        sep_ptr++;
        outlet_symbol(x->x_outfile, gensym(sep_ptr));
        outlet_symbol(x->x_outpath, gensym(x->x_mem));
      }
    }
    else
    {
      outlet_symbol(x->x_outfile, &s_);
      outlet_symbol(x->x_outpath, s);
    }
  }
}

static void *splitfilename_new(t_symbol *s, int ac, t_atom *av)
{
  t_splitfilename *x = (t_splitfilename *)pd_new(splitfilename_class);
  
  x->x_sep[0] = 0;
  x->x_sep[1] = 0;
  if(ac == 0)
    x->x_sep[0] = '/';
  else
    splitfilename_separator(x, s, ac, av);
  x->x_outpath = (t_outlet *)outlet_new(&x->x_obj, &s_symbol);
  x->x_outfile = (t_outlet *)outlet_new(&x->x_obj, &s_symbol);
  return (x);
}

void splitfilename_setup(void)
{
  splitfilename_class = class_new(gensym("splitfilename"), (t_newmethod)splitfilename_new,
    0, sizeof(t_splitfilename), 0, A_GIMME, 0);
  class_addsymbol(splitfilename_class, splitfilename_symbol);
  class_addmethod(splitfilename_class, (t_method)splitfilename_separator, gensym("separator"), A_GIMME, 0);
  class_addmethod(splitfilename_class, (t_method)splitfilename_separator, gensym("sep"), A_GIMME, 0);
//  class_sethelpsymbol(splitfilename_class, gensym("iemhelp/help-splitfilename"));
}
