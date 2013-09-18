/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iemlib2 written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"


/* --------------------------- bpe -------------------------------- */
/* -- break-point-envelope, convert a list of value-time-doubles -- */
/* ------- into a time-scheduled stream of value-time-pairs ------- */

static t_class *bpe_class;

typedef struct _bpe
{
  t_object x_obj;
  t_clock  *x_clock;
  int      x_maxnum;
  int      x_curnum;
  int      x_curindex;
  t_atom   *x_beg;
  void     *x_out_val;
  void     *x_out_time;
  void     *x_out_finished;
} t_bpe;

static void bpe_stop(t_bpe *x)
{
  clock_unset(x->x_clock);
}

static void bpe_tick(t_bpe *x)
{
  t_atom *vec = x->x_beg;
  t_float val, ftime;
  
  if(x->x_curindex >= x->x_curnum)
  {
    bpe_stop(x);
    outlet_bang(x->x_out_finished);
  }
  else
  {
    vec += x->x_curindex;
    val = atom_getfloat(vec++);
    ftime = atom_getfloat(vec);
    outlet_float(x->x_out_time, ftime);
    outlet_float(x->x_out_val, val);
    x->x_curindex += 2;
    clock_delay(x->x_clock, ftime);
  }
}

static void bpe_bang(t_bpe *x)
{
  t_atom *vec = x->x_beg;
  t_float val, ftime;
  
  if(x->x_curnum)
  {
    x->x_curindex = 2;
    val = atom_getfloat(vec++);
    ftime = atom_getfloat(vec);
    outlet_float(x->x_out_time, ftime);
    outlet_float(x->x_out_val, val);
    clock_delay(x->x_clock, ftime);
  }
}

static void bpe_list(t_bpe *x, t_symbol *s, int ac, t_atom *av)
{
  int n = ac & 0xfffffffe, i;
  t_atom *vec = x->x_beg;
  if(n > x->x_maxnum)
  {
    freebytes(x->x_beg, x->x_maxnum*sizeof(t_atom));
    x->x_maxnum = 2 + n;
    x->x_beg = (t_atom *)getbytes(x->x_maxnum*sizeof(t_atom));
    vec = x->x_beg;
  }
  x->x_curnum = n;
  for(i=0; i<n; i++)
  {
    *vec++ = *av++;
  }
}

static void bpe_free(t_bpe *x)
{
  freebytes(x->x_beg, x->x_maxnum*sizeof(t_atom));
  clock_free(x->x_clock);
}

static void *bpe_new(void)
{
  t_bpe *x = (t_bpe *)pd_new(bpe_class);
  
  x->x_curindex = 0;
  x->x_maxnum = 20;
  x->x_curnum = 0;
  x->x_beg = (t_atom *)getbytes(x->x_maxnum*sizeof(t_atom));
  x->x_clock = clock_new(x, (t_method)bpe_tick);
  x->x_out_val = outlet_new(&x->x_obj, &s_float);
  x->x_out_time = outlet_new(&x->x_obj, &s_float);
  x->x_out_finished = outlet_new(&x->x_obj, &s_bang);
  return (x);
}

void bpe_setup(void)
{
  bpe_class = class_new(gensym("bpe"), (t_newmethod)bpe_new,
    (t_method)bpe_free, sizeof(t_bpe), 0, 0);
  class_addmethod(bpe_class, (t_method)bpe_stop, gensym("stop"), 0);
  class_addbang(bpe_class, bpe_bang);
  class_addlist(bpe_class, (t_method)bpe_list);
//  class_sethelpsymbol(bpe_class, gensym("iemhelp/help-bpe"));
}
