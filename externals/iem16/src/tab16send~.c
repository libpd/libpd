/* copyleft (c) 2003 forum::für::umläute -- IOhannes m zmölnig @ IEM
 * based on d_array.c from pd:
 * Copyright (c) 1997-1999 Miller Puckette and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* tab16write~, tab16play~, tab16read~, tab16read4~, tab16send~, tab16receive~ */

#include "iem16_table.h"


/* ------------------------ tab16send~ ------------------------- */

static t_class *tab16send_class;

typedef struct _tab16send{
  t_object x_obj;
  t_iem16_16bit *x_vec;
  int x_graphperiod;
  int x_graphcount;
  t_symbol *x_arrayname;
  float x_f;
} t_tab16send;

static void *tab16send_new(t_symbol *s){
  t_tab16send *x = (t_tab16send *)pd_new(tab16send_class);
  x->x_graphcount = 0;
  x->x_arrayname = s;
  x->x_f = 0;
  return (x);
}

static t_int *tab16send_perform(t_int *w){
  t_tab16send *x = (t_tab16send *)(w[1]);
  t_float *in = (t_float *)(w[2]);
  int n = w[3];
  t_iem16_16bit *dest = x->x_vec;
  int i = x->x_graphcount;
  if (!x->x_vec) goto bad;

  while (n--)	*dest = *in++*IEM16_SCALE_UP;
  if (!i--)i = x->x_graphperiod;
  x->x_graphcount = i;
 bad:
  return (w+4);
}

static void tab16send_dsp(t_tab16send *x, t_signal **sp){
  int vecsize;
  t_table16 *a;

  if (!(a = (t_table16 *)pd_findbyclass(x->x_arrayname, table16_class)))    {
    if (*x->x_arrayname->s_name)
      error("tab16send~: %s: no such array", x->x_arrayname->s_name);
  }
  else if (!table16_getarray16(a, &vecsize, &x->x_vec))
    error("%s: bad template for tab16send~", x->x_arrayname->s_name);
  else    {
    int n = sp[0]->s_n;
    int ticksper = sp[0]->s_sr/n;
    if (ticksper < 1) ticksper = 1;
    x->x_graphperiod = ticksper;
    if (x->x_graphcount > ticksper) x->x_graphcount = ticksper;
    if (n < vecsize) vecsize = n;
    table16_usedindsp(a);
    dsp_add(tab16send_perform, 3, x, sp[0]->s_vec, vecsize);
  }
}

static void tab16send_free(t_tab16send *x){}

static void tab16send_setup(void){
  tab16send_class = class_new(gensym("tab16send~"), (t_newmethod)tab16send_new,
			      (t_method)tab16send_free, sizeof(t_tab16send), 0, A_DEFSYM, 0);
  CLASS_MAINSIGNALIN(tab16send_class, t_tab16send, x_f);
  class_addmethod(tab16send_class, (t_method)tab16send_dsp, gensym("dsp"), 0);
}

// G.Holzmann: for PD-extended build system
void tab16send_tilde_setup(void)
{
  tab16send_setup();
}
