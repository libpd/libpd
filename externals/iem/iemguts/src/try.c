
/******************************************************
 *
 * try - implementation file
 *
 * copyleft (c) IOhannes m zmölnig
 *
 *   2007:forum::für::umläute:2007
 *
 *   institute of electronic music and acoustics (iem)
 *
 ******************************************************
 *
 * license: GNU General Public License v.2
 *
 ******************************************************/


/* 
 * this object provides a way to create an object with a fallback
 * [try bla 13, blu 134] will first try to create an obect [bla 13] and if this fails use [blu 134] instead.
 *
 * currently this only works for objectclasses (no abstractions)
 * currently this doesn't work (well) with [list]  
 */

#include "m_pd.h"
#include "g_canvas.h"

int glist_getindex(t_glist *x, t_gobj *y);

/* ------------------------- try ---------------------------- */

static t_class *try_class;

typedef struct _try
{
  t_object  x_obj;
} t_try;


typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);

t_pd*try_this(int argc, t_atom*argv) {
  t_symbol*s=NULL;
  if(!argc)return NULL;

  s=atom_getsymbol(argv);
  if(A_SYMBOL==argv->a_type) {
    argc--; 
    argv++;
  }

  //startpost("[%s] (%x): ", s->s_name, s); postatom(argc, argv); endpost();

  t_newgimme fun=(t_newgimme)zgetfn(&pd_objectmaker, s);
  if(fun) {
    //post("found a creator for [%s]", s->s_name);
    return fun(s, argc, argv);
  }

  return NULL;
}

static void *try_new(t_symbol*s, int argc, t_atom*argv)
{
  t_pd*x=NULL;
  int start=0, i=0;
  if(!pd_objectmaker) { 
    error("[try] could not find pd_objectmaker");
    return NULL;
  }

  for(i=0; i<argc; i++) {
    if(atom_getsymbolarg(i,argc,  argv)==gensym(",")) {
      x=try_this(i-start, argv+start);
      if(x)return x;
      start=i+1;
    }
  }

  x=try_this(argc-start, argv+start);

  return (x);
}

void try_setup(void)
{
  try_class = class_new(gensym("try"), 
			(t_newmethod)try_new, NULL, 
			sizeof(t_try), 0, 
			A_GIMME, 0);
}
