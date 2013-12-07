/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_t3_lib written by Gerhard Eckel, Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"

/* ------------------------ t3_bpe ---------------------------- */
static t_class *t3_bpe_class;

typedef struct _t3_bpe
{
  t_object x_obj;
  t_atom   x_at[2];
  t_clock  *x_clock;
  int      x_maxnum;
  int      x_curnum;
  int      x_curindex;
  t_atom   *x_beg;
  double   x_t3_bang;
  double   x_ticks2ms;
  int      x_hit;
  void     *x_out_val;
  void     *x_out_time;
  void     *x_out_finished;
} t_t3_bpe;

static void t3_bpe_stop(t_t3_bpe *x)
{
  clock_unset(x->x_clock);
}

static void t3_bpe_tick(t_t3_bpe *x)
{
  t_atom *vec = x->x_beg;
  t_float val;
  double dticks, dtime;
  int iticks;
  
  if(x->x_curindex >= x->x_curnum)
  {
    t3_bpe_stop(x);
    outlet_float(x->x_out_finished, x->x_t3_bang);
  }
  else
  {
    x->x_hit = 0;
    vec += x->x_curindex;
    val = atom_getfloat(vec++);
    dtime = (double)atom_getfloat(vec);
    outlet_float(x->x_out_time, dtime);
    x->x_at[1].a_w.w_float = val;
    x->x_at[0].a_w.w_float = x->x_t3_bang;
    outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
    dticks = (dtime + x->x_t3_bang)/x->x_ticks2ms;
    iticks = (int)dticks;
    x->x_t3_bang = (dticks - (double)iticks)*x->x_ticks2ms;
    if(!x->x_hit)
      clock_delay(x->x_clock, (double)iticks*x->x_ticks2ms);
    x->x_curindex += 2;
  }
}

static void t3_bpe_float(t_t3_bpe *x, t_floatarg f)
{
  double dticks;
  int iticks;
  
  if(x->x_curnum)
  {
    x->x_curindex = 0;
    dticks = (double)f/x->x_ticks2ms;
    iticks = (int)dticks;
    x->x_t3_bang = (dticks - (double)iticks)*x->x_ticks2ms;
    clock_delay(x->x_clock, (double)iticks*x->x_ticks2ms);
    x->x_hit = 1;
  }
}

static void t3_bpe_list(t_t3_bpe *x, t_symbol *s, int ac, t_atom *av)
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

static void t3_bpe_free(t_t3_bpe *x)
{
  freebytes(x->x_beg, x->x_maxnum*sizeof(t_atom));
  clock_free(x->x_clock);
}

static void *t3_bpe_new(void)
{
  t_t3_bpe *x = (t_t3_bpe *)pd_new(t3_bpe_class);
  
  x->x_t3_bang = 0.0;
  x->x_ticks2ms = 1000.0*(double)sys_getblksize()/(double)sys_getsr();
  x->x_curindex = 0;
  x->x_maxnum = 20;
  x->x_curnum = 0;
  x->x_hit = 0;
  x->x_beg = (t_atom *)getbytes(x->x_maxnum*sizeof(t_atom));
  x->x_clock = clock_new(x, (t_method)t3_bpe_tick);
  outlet_new(&x->x_obj, &s_list);
  x->x_out_time = outlet_new(&x->x_obj, &s_float);
  x->x_out_finished = outlet_new(&x->x_obj, &s_float);
  x->x_at[0].a_type = A_FLOAT;
  x->x_at[1].a_type = A_FLOAT;
  return (x);
}

void t3_bpe_setup(void)
{
  t3_bpe_class = class_new(gensym("t3_bpe"), (t_newmethod)t3_bpe_new,
    (t_method)t3_bpe_free, sizeof(t_t3_bpe), 0, 0);
  class_addmethod(t3_bpe_class, (t_method)t3_bpe_stop, gensym("stop"), 0);
  class_addfloat(t3_bpe_class, (t_method)t3_bpe_float);
  class_addlist(t3_bpe_class, (t_method)t3_bpe_list);
//  class_sethelpsymbol(t3_bpe_class, gensym("iemhelp/help-t3_bpe"));
}
