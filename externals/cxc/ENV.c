/*
  cxc@web.fm, 2002 02
 */

#include "m_pd.h"
#include <stdlib.h>

#ifdef _WIN32
#define setenv(a,b,c) _putenv(a)
#endif /* _WIN32 */

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

t_class *ENV_class;

typedef struct ENV
{
  t_object x_obj;
  t_float  x_RM;
  t_atom   x_at[1];
} t_ENV;

void ENV_float(t_ENV *x, t_floatarg f)
{
   post("cxc/ENV.c: a float %f", f);
}

void ENV_RAND_MAX(t_ENV *x)
{
  SETFLOAT(x->x_at, x->x_RM);
#ifdef DEBUG
  post("cxc/ENV.c: %f",atom_getfloat(x->x_at));
#endif
  outlet_list(x->x_obj.ob_outlet, &s_list, 1,x->x_at);
}

void ENV_getenv(t_ENV *x, t_symbol *s)
{
  char *env;
  env = getenv(s->s_name);
#ifdef DEBUG
  post("cxc/ENV.c: %s",env);
#endif
  if(env!=NULL)
    SETSYMBOL(x->x_at,gensym(env));
  else
    SETFLOAT(x->x_at,-1);
  outlet_list(x->x_obj.ob_outlet, &s_list, 1,x->x_at);
}

void ENV_setenv(t_ENV *x, t_symbol *s, t_symbol *t)
{
  //t_symbol *t = atom_getsymbol(a);
  // post("cxc/ENV.c: %s=%s",s->s_name,t->s_name);
  if(setenv(s->s_name,t->s_name,1)!=-1)
    post("cxc/ENV.c: set %s=%s",s->s_name,t->s_name);
  else
    post("cxc/ENV.c: set failed");
}

void *ENV_new(void)
{
    t_ENV *x = (t_ENV *)pd_new(ENV_class);
    x->x_RM  = RAND_MAX;
    //x->x_at  = (t_atom;
    outlet_new(&x->x_obj, &s_float);
    //post("ENV_new");
    return (void *)x;
}

void ENV_help(void)
{
  post("cxc/ENV.c: get + set ENVironment variables");
}

void ENV_setup(void)
{
  // post("ENV_setup");
    ENV_class = class_new(gensym("ENV"), (t_newmethod)ENV_new, 0,
    	sizeof(t_ENV), 0, 0);
    class_addmethod(ENV_class, (t_method)ENV_RAND_MAX, gensym("RAND_MAX"), 0);
    class_addmethod(ENV_class, (t_method)ENV_getenv, gensym("getenv"), A_SYMBOL);
    class_addmethod(ENV_class, (t_method)ENV_setenv, gensym("setenv"), A_SYMBOL, A_SYMBOL);
    class_addfloat(ENV_class, ENV_float);
}

