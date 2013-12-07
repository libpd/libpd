/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */


#include "m_pd.h"
#include "iemlib.h"


/* ------------------------ post_netreceive ---------------------------- */
static t_class *post_netreceive_class;

typedef struct _post_netreceive
{
  t_object   x_obj;
  int        x_max_send_entries;
  int        x_min_send_entries;
  char       *x_snd_able;
  t_symbol   **x_send_entries;
  t_symbol   **x_plus_entries;
  t_symbol   *x_set;
  t_atom     x_at[2];
  t_atom     *x_atbuf;
  t_symbol   *x_send;
  t_symbol   *x_setall;
} t_post_netreceive;

static void post_netreceive_list(t_post_netreceive *x, t_symbol *s, int ac, t_atom *av)
{
  int identifier_index = (int)atom_getintarg(0, ac, av);
  t_symbol *sender=0;
  
  if((identifier_index >= x->x_min_send_entries) && (identifier_index < x->x_max_send_entries))
  {
    if(x->x_snd_able[identifier_index] == 1)
    {
      sender = x->x_send_entries[identifier_index];
      if(sender->s_thing)
      {
        if(ac == 2)
          pd_float(sender->s_thing, atom_getfloatarg(1, ac, av));
        else
          pd_list(sender->s_thing, &s_list, ac-1, av+1);
      }
    }
    else if(x->x_snd_able[identifier_index] == 2)
    {
      sender = x->x_send_entries[identifier_index];
      if(sender->s_thing)
      {
        typedmess(sender->s_thing, x->x_plus_entries[identifier_index], ac-1, av+1);
      }
    }
    if(IS_A_FLOAT(av,1))
      SETFLOAT(x->x_atbuf+identifier_index+1, atom_getfloatarg(1, ac, av));
    else if(IS_A_SYMBOL(av,1))
      SETSYMBOL(x->x_atbuf+identifier_index+1, atom_getsymbolarg(1, ac, av));
  }
}

/*static void post_netreceive_set_item_name(t_post_netreceive *x, t_symbol *snd, t_float findex)
{
int index = (int)findex;

  if((index >= 0) && (index < x->x_max_send_entries))
  {
  x->x_send_entries[index] = snd;
  x->x_snd_able[index] = 1;
  }
}*/

static void post_netreceive_set_item_name(t_post_netreceive *x, t_symbol *s, int ac, t_atom *av)
{
  if(ac == 2)
  {
    int identifier_index = (int)atom_getintarg(1, ac, av);
    
    if((identifier_index >= x->x_min_send_entries) && (identifier_index < x->x_max_send_entries))
    {
      x->x_send_entries[identifier_index] = atom_getsymbolarg(0, ac, av);
      x->x_snd_able[identifier_index] = 1;
    }
  }
  else if(ac >= 3)
  {
    int identifier_index = (int)atom_getintarg(2, ac, av);
    
    if((identifier_index >= 0) && (identifier_index < x->x_max_send_entries))
    {
      x->x_send_entries[identifier_index] = atom_getsymbolarg(1, ac, av);
      x->x_snd_able[identifier_index] = 2;
      x->x_plus_entries[identifier_index] = atom_getsymbolarg(0, ac, av);
    }
  }
}

static void post_netreceive_all_parameters(t_post_netreceive *x, t_symbol *s, int ac, t_atom *av)
{
  int i, min = x->x_min_send_entries, max = x->x_max_send_entries;
  t_symbol *sendname=0;
  
  if(min > ac)
    min = ac;
  if(max > ac)
    max = ac;
  for(i=min; i<max; i++)
  {
    if(x->x_snd_able[i] == 1)
    {
      sendname = x->x_send_entries[i];
      if(sendname->s_thing)
      {
        pd_float(sendname->s_thing, atom_getfloatarg(i, ac, av));
      }
    }
    else if(x->x_snd_able[i] == 2)
    {
      sendname = x->x_send_entries[i];
      if(sendname->s_thing)
      {
        typedmess(sendname->s_thing, x->x_plus_entries[i], 1, av+i);
      }
    }
    if(IS_A_FLOAT(av,i))
      SETFLOAT(x->x_atbuf+i+1, atom_getfloatarg(i, ac, av));
    else if(IS_A_SYMBOL(av,i))
      SETSYMBOL(x->x_atbuf+i+1, atom_getsymbolarg(i, ac, av));
  }
}

static void post_netreceive_set_all_parameters(t_post_netreceive *x, t_symbol *s, int ac, t_atom *av)
{
  int i, min = x->x_min_send_entries, max = x->x_max_send_entries;
  t_symbol *sendname;
  
  if(min > ac)
    min = ac;
  if(max > ac)
    max = ac;
  for(i=min; i<max; i++)
  {
    if(x->x_snd_able[i])
    {
      sendname = x->x_send_entries[i];
      if(sendname->s_thing)
      {
        typedmess(sendname->s_thing, x->x_set, 1, av+i);
      }
    }
  }
}

static void post_netreceive_clear(t_post_netreceive *x)
{
  int i, max = x->x_max_send_entries;
  
  for(i=0; i<max; i++)
  {
    if(x->x_snd_able[i])
    {
      x->x_snd_able[i] = 0;
    }
  }
}

static void post_netreceive_fetch_all_parameters(t_post_netreceive *x, t_floatarg nr_sended_para)
{
  int nrsp=(int)nr_sended_para;
  t_atom *ap=x->x_atbuf+x->x_min_send_entries;
  
  if(nrsp <= 0)
    nrsp = 1;
  if(nrsp > (x->x_max_send_entries - x->x_min_send_entries))
    nrsp = (x->x_max_send_entries - x->x_min_send_entries);
  outlet_anything(x->x_obj.ob_outlet, x->x_send, nrsp+1, x->x_atbuf);
}

static void post_netreceive_free(t_post_netreceive *x)
{
  freebytes(x->x_snd_able, x->x_max_send_entries * sizeof(char));
  freebytes(x->x_send_entries, x->x_max_send_entries * sizeof(t_symbol *));
  freebytes(x->x_atbuf, (x->x_max_send_entries+2) * sizeof(t_atom));
  freebytes(x->x_plus_entries, x->x_max_send_entries * sizeof(t_symbol *));
}

static void *post_netreceive_new(t_floatarg fmin, t_floatarg fmax)
{
  t_post_netreceive *x = (t_post_netreceive *)pd_new(post_netreceive_class);
  int i, min = (int)fmin, max = (int)fmax;
  t_atom *ap;
  
  if(min < 0)
    min = 0;
  if(max <= 0)
    max = 80;
  x->x_min_send_entries = min;
  x->x_max_send_entries = max;
  x->x_atbuf = (t_atom *)getbytes((x->x_max_send_entries+2) * sizeof(t_atom));
  x->x_snd_able = (char *)getbytes(x->x_max_send_entries * sizeof(char));
  x->x_send_entries = (t_symbol **)getbytes(x->x_max_send_entries * sizeof(t_symbol *));
  x->x_plus_entries = (t_symbol **)getbytes(x->x_max_send_entries * sizeof(t_symbol *));
  x->x_set = gensym("set");
  x->x_setall = gensym("set_all_parameters");
  x->x_send = gensym("send");
  ap = x->x_atbuf;
  SETSYMBOL(ap, x->x_setall);
  ap++;
  for(i=1; i<=max; i++)
  {
    SETFLOAT(ap, 0.0);
    ap++;
  }
  for(i=0; i<max; i++)
  {
    x->x_snd_able[i] = 0;
    x->x_plus_entries[i] = x->x_set;
  }
  outlet_new(&x->x_obj, &s_list);
  return (x);
}

void post_netreceive_setup(void)
{
  post_netreceive_class = class_new(gensym("post_netreceive"), (t_newmethod)post_netreceive_new,
    (t_method)post_netreceive_free, sizeof(t_post_netreceive), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addlist(post_netreceive_class, post_netreceive_list);
  class_addmethod(post_netreceive_class, (t_method)post_netreceive_set_item_name, gensym("set_item_name"), A_GIMME, 0);
  class_addmethod(post_netreceive_class, (t_method)post_netreceive_all_parameters, gensym("all_parameters"), A_GIMME, 0);
  class_addmethod(post_netreceive_class, (t_method)post_netreceive_set_all_parameters, gensym("set_all_parameters"), A_GIMME, 0);
  class_addmethod(post_netreceive_class, (t_method)post_netreceive_fetch_all_parameters, gensym("fetch_all_parameters"), A_DEFFLOAT, 0);
  class_addmethod(post_netreceive_class, (t_method)post_netreceive_clear, gensym("clear"), 0);
//  class_sethelpsymbol(post_netreceive_class, gensym("iemhelp/help-post_netreceive"));
}
