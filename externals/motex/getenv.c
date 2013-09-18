/*************************************************************************** 
 * File: shuffle.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd external. Sends value of an environment variable argument on bang.
 * Use a 'set <NAME>' message to reset the variable name.
 * See supporting Pd patch: getenv.pd
 * 
 * Copyright (C) 2001 by Iain Mott [iain.mott@bigpond.com] 
 * 
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2, or (at your option) 
 * any later version. 
 * 
 * This program is distributed in the hope that it will be useful, 
 * but WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * GNU General Public License, which should be included with this 
 * program, for more details. 
 * 
 ****************************************************************************/ 

/*   
 * getenv - Pd external. Copyright (c) 2001 Iain Mott 
 * sends value of an environment variable argument on bang
 * use a 'set <NAME>' message to reset the variable name
*/

#include "m_pd.h"
#include <stdlib.h>

static t_class *getenv_class;

typedef struct _getenv
{
  t_object x_obj;
  t_symbol *enval;
  char *envar;
} t_getenv;

static void *getenv_new(t_symbol *s, int argc, t_atom *argv)
{
  
  t_getenv *x = (t_getenv *)pd_new(getenv_class);
  x->envar = NULL;
  if(argc != 1 || argv[0].a_type != A_SYMBOL)
    post("getenv: One argument (environment variable) required");
  else
    {
      char *value = getenv(argv[0].a_w.w_symbol->s_name);
      if(value)
	{
	  x->envar = argv[0].a_w.w_symbol->s_name;
	  x->enval = gensym(value);
	}
      else
	post("getenv: Environment variable does not exist");
    }  
  outlet_new(&x->x_obj, &s_symbol);
  return (x);
}


void getenv_set(t_getenv *x, t_symbol *f)
{
  char *value = getenv(f->s_name);
  if(value)
    x->enval = gensym(value);
  else
    post("getenv: Environment variable does not exist");
}

void getenv_bang(t_getenv *x)
{
  if(x->envar)
    outlet_symbol(x->x_obj.ob_outlet, x->enval);
}

void getenv_setup(void)
{
  getenv_class = class_new(gensym("getenv"), (t_newmethod)getenv_new, 0, sizeof(t_getenv), 0, A_GIMME, 0);
  class_addmethod(getenv_class, (t_method)getenv_set, gensym("set"), A_SYMBOL, 0);  
  class_addbang(getenv_class, getenv_bang);
}











