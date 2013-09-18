/* ascii-sequencer
   input lists, strings, anythings, output every characters ascii code
   jdl@xdv.org 2002
*/

#include "m_pd.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXMSGLEN 1024
#define LISTSEL "list"

t_class *ascseq_class;

typedef struct ascseq
{
  t_object x_obj;
  t_clock *x_clock;
  double   x_targettime;
  double   x_prevtime;
  t_float  x_grain;
  char     x_bla[MAXMSGLEN];
  int      x_toklen;
  int      x_tokcur;
} t_ascseq;

static void ascseq_tick(t_ascseq *x)
{
  double timenow = clock_getsystime();
  double msectogo = - clock_gettimesince(x->x_targettime);
  int chr;
  // post("ascseq: timenow: %f",timenow);
  if(x->x_tokcur >= x->x_toklen) { 
    clock_unset(x->x_clock);
    x->x_tokcur = 0;
    return;
  } else {
    chr = x->x_bla[x->x_tokcur];
    // post("tokcur: %c",chr);
    x->x_tokcur++;
    outlet_float(x->x_obj.ob_outlet, chr);
    clock_delay(x->x_clock,x->x_grain);
  }
}

void ascseq_anything(t_ascseq *x, t_symbol* s, t_int argc, t_atom* argv)
{
  int i = argc;
  int chr, cnt, len;
  char tmp[MAXMSGLEN];
  double timenow = clock_getlogicaltime();
  
  chr = 0;
  cnt = 0;
  len = 0;
  x->x_bla[0] = '\0';

  //symbol_string(s->s_name, tmp, 1024);
  if(!strstr(s->s_name,LISTSEL)) {
    strcat(x->x_bla,s->s_name);
    // post("tmp: %s",x->x_bla);
  }

  while (i--) {
    atom_string(argv, tmp, 1024);
    strcat(x->x_bla,tmp);
    // post("ascseq.c: argsize: %d",cnt);
    // post("ascseq.c: bla: %s",bla);
    argv++;
  }
  x->x_prevtime = timenow;
  x->x_targettime = clock_getsystimeafter(x->x_grain);

  x->x_toklen = strlen(x->x_bla);
  ascseq_tick(x);

/*   for(cnt=0;cnt<len;cnt++) { */
/*     //printf("'%c'\n",bla[cnt]); */
/*     chr = x->x_bla[cnt]; */
/*     outlet_float(x->x_obj.ob_outlet, chr); */
/*   } */
}

void ascseq_symbol(t_ascseq *x, t_symbol *s)
{
  t_atom* a = NULL;
  ascseq_anything(x, s, 0, a);
}

void ascseq_float(t_ascseq *x, t_floatarg f)
{
  int chr, cnt, len;
  double timenow = clock_getlogicaltime();
  
  chr = 0;
  cnt = 0;
  len = 0;
  x->x_bla[0] = '\0';

  x->x_prevtime = timenow;
  x->x_targettime = clock_getsystimeafter(x->x_grain);
  
  sprintf(x->x_bla,"%f",f);
  x->x_toklen = strlen(x->x_bla);

  ascseq_tick(x);

/*   for(cnt=0;cnt<len;cnt++){ */
/*     // printf("'%c'\n",bla[cnt]); */
/*     chr = x->x_bla[cnt]; */
/*     outlet_float(x->x_obj.ob_outlet, chr); */
/*     // clock_delay(x->x_clock,1000.); */
    /*    } */
}

static void ascseq_ft1(t_ascseq *x, t_floatarg g)
{
    if (g < 0) g = 0;
    if (!g)    g = 0;
    x->x_grain = g;
}

void ascseq_free(t_ascseq *x)
{
  clock_free(x->x_clock);
  // post("ascseq_free");
}

void *ascseq_new(t_floatarg f)
{
    t_ascseq *x = (t_ascseq *)pd_new(ascseq_class);
    ascseq_ft1(x,f);
    x->x_clock = clock_new(x, (t_method)ascseq_tick);
    x->x_targettime = x->x_prevtime = clock_getlogicaltime();
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("ft1"));
    outlet_new(&x->x_obj, &s_float);
    return (void *)x;
}

void ascseq_setup(void)
{
  // post("ascseq_setup");
  ascseq_class = class_new(gensym("ascseq"), (t_newmethod)ascseq_new,
			   (t_method)ascseq_free, sizeof(t_ascseq), 0, A_DEFFLOAT, 0);
  class_addlist(ascseq_class, ascseq_anything);
  class_addanything(ascseq_class,ascseq_anything);
  class_addfloat(ascseq_class, ascseq_float);
  class_addmethod(ascseq_class, (t_method)ascseq_ft1, gensym("ft1"), A_FLOAT, 0);
  class_addsymbol(ascseq_class, ascseq_symbol);
}
