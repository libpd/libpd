/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* ------------------------- list2send ----------------------------- */
/* -- via an array of send labels, an incomming list with leading -- */
/* -- float index will be sent to a receive object with the label -- */
/* ------------ name of the index-th entry of the array ------------ */

static t_class *list2send_class;

typedef struct _list2send
{
  t_object    x_obj;
  int         x_max;
  char        *x_snd_able;
  t_symbol    **x_send_entries;
  t_symbol    *x_set;
} t_list2send;

static void list2send_list(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 2) && (IS_A_FLOAT(av,0)))
  {
    int identifier_index = (int)atom_getintarg(0, ac, av);
    
    if(identifier_index < x->x_max)
    {
      if(x->x_snd_able[identifier_index])
      {
        t_symbol *sender = x->x_send_entries[identifier_index];
        
        if(sender->s_thing)
        {
          if(ac == 2)
          {
            if(IS_A_FLOAT(av, 1))
              pd_float(sender->s_thing, atom_getfloatarg(1, ac, av));
            else if(IS_A_SYMBOL(av, 1))
              pd_symbol(sender->s_thing, atom_getsymbolarg(1, ac, av));
          }
          else
            pd_list(sender->s_thing, &s_list, ac-1, av+1);
        }
      }
    }
  }
}

static void list2send_set(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac >= 2) && (IS_A_FLOAT(av,0)))
  {
    int identifier_index = (int)atom_getintarg(0, ac, av);
    
    if(identifier_index < x->x_max)
    {
      if(x->x_snd_able[identifier_index])
      {
        t_symbol *sender = x->x_send_entries[identifier_index];
        
        if(sender->s_thing)
          typedmess(sender->s_thing, s, ac-1, av+1);
      }
    }
  }
}

static void list2send_all(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  int i, n=x->x_max;
  
  if(ac == n)
  {
    for(i=n-1; i>=0; i--)/*change*/
    {
      if(x->x_snd_able[i])
      {
        t_symbol *sender = x->x_send_entries[i];
        
        if(sender->s_thing)
        {
          if(IS_A_FLOAT(av, i))
            pd_float(sender->s_thing, atom_getfloatarg(i, ac, av));
          else if(IS_A_SYMBOL(av, i))
            pd_symbol(sender->s_thing, atom_getsymbolarg(i, ac, av));
        }
      }
    }
  }
}

static void list2send_set_all(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  int i, n=x->x_max;
  
  if(ac == n)
  {
    for(i=n-1; i>=0; i--)/*change*/
    {
      if(x->x_snd_able[i])
      {
        t_symbol *sender = x->x_send_entries[i];
        
        if(sender->s_thing)
          typedmess(sender->s_thing, x->x_set, 1, av+i);
      }
    }
  }
}

static void list2send_from(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  int n=x->x_max;
  
  if(ac >= 1)
  {
    int i, j, beg=(int)atom_getintarg(0, ac, av);
    
    if((beg + ac - 1) <= n)
    {
      for(i=ac-1,j=beg+ac-2; i>=1; i--,j--)/*change*/
      {
        if(x->x_snd_able[j])
        {
          t_symbol *sender = x->x_send_entries[j];
          
          if(sender->s_thing)
          {
            if(IS_A_FLOAT(av, i))
              pd_float(sender->s_thing, atom_getfloatarg(i, ac, av));
            else if(IS_A_SYMBOL(av, i))
              pd_symbol(sender->s_thing, atom_getsymbolarg(i, ac, av));
          }
        }
      }
    }
  }
}

static void list2send_set_from(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  int n=x->x_max;
  
  if(ac >= 1)
  {
    int i, j, beg=(int)atom_getintarg(0, ac, av);
    
    if((beg + ac - 1) <= n)
    {
      for(i=ac-1,j=beg+ac-2; i>=1; i--,j--)/*change*/
      {
        if(x->x_snd_able[j])
        {
          t_symbol *sender = x->x_send_entries[j];
          
          if(sender->s_thing)
            typedmess(sender->s_thing, x->x_set, 1, av+i);
        }
      }
    }
  }
}

static void list2send_add(t_list2send *x, t_symbol *s, int ac, t_atom *av)
{
  if((ac > 1) && (IS_A_FLOAT(av, 0)))
  {
    int identifier_index = (int)atom_getintarg(0, ac, av);
    
    if(identifier_index < x->x_max)
    {
      if(IS_A_SYMBOL(av, 1))
      {
        x->x_send_entries[identifier_index] = atom_getsymbolarg(1, ac, av);
        x->x_snd_able[identifier_index] = 1;
      }
      else if(IS_A_FLOAT(av, 1))
      {
        char str[100];
        
        sprintf(str, "%g", atom_getfloatarg(1, ac, av));
        x->x_send_entries[identifier_index] = gensym(str);
        x->x_snd_able[identifier_index] = 1;
      }
    }
  }
}

static void list2send_clear(t_list2send *x)
{
  int i, n=x->x_max;
  
  for(i=0; i<n; i++)
  {
    if(x->x_snd_able[i])
      x->x_snd_able[i] = 0;
  }
}

static void list2send_free(t_list2send *x)
{
  freebytes(x->x_snd_able, x->x_max * sizeof(char));
  freebytes(x->x_send_entries, x->x_max * sizeof(t_symbol *));
}

static void *list2send_new(t_floatarg fmax)
{
  t_list2send *x = (t_list2send *)pd_new(list2send_class);
  int i, max = (int)fmax;
  t_atom *ap;
  
  if(max <= 0)
    max = 80;
  x->x_max = max;
  x->x_snd_able = (char *)getbytes(max * sizeof(char));
  x->x_send_entries = (t_symbol **)getbytes(max * sizeof(t_symbol *));
  for(i=0; i<max; i++)
    x->x_snd_able[i] = 0;
  x->x_set = gensym("set");
  return (x);
}

void list2send_setup(void)
{
  list2send_class = class_new(gensym("list2send"), (t_newmethod)list2send_new,
    (t_method)list2send_free, sizeof(t_list2send), 0, A_DEFFLOAT, 0);
  class_addlist(list2send_class, list2send_list);
  class_addmethod(list2send_class, (t_method)list2send_add, gensym("add"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_set, gensym("set"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_all, gensym("all"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_set_all, gensym("set_all"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_from, gensym("from"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_set_from, gensym("set_from"), A_GIMME, 0);
  class_addmethod(list2send_class, (t_method)list2send_clear, gensym("clear"), 0);
//  class_sethelpsymbol(list2send_class, gensym("iemhelp/help-list2send"));
}
