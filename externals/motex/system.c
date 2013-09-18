/*************************************************************************** 
 * File: system.c 
 * Auth: Iain Mott [iain.mott@bigpond.com] 
 * Maintainer: Iain Mott [iain.mott@bigpond.com] 
 * Version: Part of motex_1.1.2 
 * Date: January 2001
 * 
 * Description: Pd external. Send a system message to the console 
 * See supporting Pd patch: system.pd
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

/* code for system pd class */

#include "m_pd.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
 
typedef struct system
{
  t_object t_ob;
} t_system;


void system_anything(t_system *x, t_symbol *s, int ac, t_atom *av, t_floatarg f)
{
  char buf[MAXPDSTRING], message[MAXPDSTRING];
  int i;
  strcpy(message, s->s_name);
  strcat(message, " ");
  for (i = 0; i < ac; i++)
    {
      atom_string(av+i, buf, MAXPDSTRING);
      strcat(message, buf);
      if (i < (ac - 1))
	strcat(message, " ");
    }
  system(message);
}


void system_free(void)
{
  /*      post("system_free"); */
}

t_class *system_class;

void *system_new(void)
{
  t_system *x = (t_system *)pd_new(system_class);
  return (void *)x;
}

void system_setup(void)
{
//  post("system_setup");
  system_class = class_new(gensym("system"), (t_newmethod)system_new,
			   (t_method)system_free, sizeof(t_system), 0, 0);
  class_addanything(system_class, system_anything);
}

