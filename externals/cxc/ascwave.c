/* code for foo1 pd class */

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#ifdef _WIN32
#include <malloc.h>
#endif

#ifndef RAND_MAX
#define	RAND_MAX	21
#endif

t_class *ascwave_class;
 
typedef struct ascwave
{
  t_object t_ob;
  t_symbol* filename;
  FILE*  x_file;
  // width vertical, sort of useless
  t_float x_jodel;
  // width horizontal
  t_float x_julp;
  // fill or not flag
  t_float x_fill;
  // chr to use for draw
  t_float x_chr;
  // chr to use 4 fill
} t_ascwave;

void ascwave_bang(t_ascwave *x, t_floatarg f)
{
  // post("cxc/ascwave.c: bang %f", x->x_jodel);
  outlet_float(x->t_ob.ob_outlet, x->x_jodel + x->x_jodel);
}

/* fill or line toggle */
static void ascwave_fill(t_ascwave *x, t_floatarg f)
{
  x->x_fill = f;
  //  post("ascwave: fill %f", x->x_fill);
}


/* open a file to put ascii output into */
static void ascwave_open(t_ascwave *x, t_symbol *filename)
{
     post("ascwave: open");
     x->filename = filename;
     post("ascwave: filename = %s",x->filename->s_name);
     if ((x->x_file = sys_fopen(x->filename->s_name,"w")) < 0)
     {
	  error("can't create %s",filename->s_name);
	  return;
     }
}


void ascwave_ft1(t_ascwave *x, t_floatarg g)
{
  int sz = x->x_julp;
  int lchr = x->x_chr;
  int schr = 32;
  int i = 0;
  char* xip;
  char* xap;
  xip = (char*)malloc((sz+1)*sizeof(char));
  xap = (char*)malloc((sz+1)*sizeof(char));

  for (i = 0;i <= sz; ++i) {
    if (i == sz-1) {
      xip[i] = lchr;
    } else {
      if (!x->x_fill) {
	xip[i] = schr;
      } else {
	xip[i] = lchr;
	/*	if (rand() > 20)
		xip[i] = '\n'; */
      }
    }
    if (i == 0 || i == sz-1)
      xap[i] = lchr;
    else
      xap[i] = i % 80 + 33;
  }
  //  xip[sz] = schr;//'\n';
  xip[sz+1] = '\0';
  //xap[sz] = schr;//'\n';
  xap[sz+1] = '\0';
  //  poststring(xip);
  //  post("ft1: %f, %d", x->x_jodel, sz);
  //  outlet_float(x->t_ob.ob_outlet, x->x_jodel + x->x_jodel);
  outlet_symbol(x->t_ob.ob_outlet, gensym(xip));
  for (i = 0; i < g-2;++i)
    outlet_symbol(x->t_ob.ob_outlet, gensym(xap));
  if (g > 1)
    outlet_symbol(x->t_ob.ob_outlet, gensym(xip));
  x->x_jodel = g;

  free(xip);
  free(xap);
}

static void ascwave_width(t_ascwave *x, t_floatarg g)
{
  if (g < 0)
    x->x_julp = 0;
  else
    x->x_julp = g;
  //post("ascwave: setting width: %f", x->x_julp);
}


static void ascwave_chr(t_ascwave *x, t_floatarg g)
{
  x->x_chr = g;
  //  post("ascwave: setting character: %f", x->x_chr);
}


void ascwave_free() { }

void *ascwave_new()
{
    t_ascwave *x = (t_ascwave *)pd_new(ascwave_class);
    x->x_chr = 46;
    //    outlet_new(&x->t_ob, &s_float);
    outlet_new(&x->t_ob, &s_symbol);
    inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft1"));
    inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft2"));
    inlet_new(&x->t_ob, &x->t_ob.ob_pd, gensym("float"), gensym("ft3"));
    // post("ascwave_new");
    return (void *)x;
}

void ascwave_setup()
{
  // post("ascwave_setup");
    ascwave_class = class_new(gensym("ascwave"), (t_newmethod)ascwave_new, 0,
    	sizeof(t_ascwave), 0, 0);
    class_addmethod(ascwave_class, (t_method)ascwave_bang, gensym("bang"), 0);
    class_addmethod(ascwave_class, (t_method)ascwave_fill, gensym("fill"), A_FLOAT, A_NULL);
    class_addmethod(ascwave_class, (t_method) ascwave_open, gensym("open"), A_SYMBOL,A_NULL);
    class_addmethod(ascwave_class, (t_method)ascwave_ft1, gensym("ft1"), A_FLOAT, 0);
    class_addmethod(ascwave_class, (t_method)ascwave_width, gensym("ft2"), A_FLOAT, 0);
    // set chr
    class_addmethod(ascwave_class, (t_method)ascwave_chr, gensym("ft3"), A_FLOAT, 0);
}
