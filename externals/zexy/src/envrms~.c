/* 
 *  envrms~: simple envelope follower
 *
 * (c) 1999-2011 IOhannes m zmölnig, forum::für::umläute, institute of electronic music and acoustics (iem)
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* ---------------- envrms~ - simple envelope follower. ----------------- */
/* this is exactly the same as msp's env~-object, but does not output dB but RMS !! */
/* i found env~+dbtorms most inconvenient (and expensive...) */

#include "zexy.h"

#define MAXOVERLAP 10
#define MAXVSTAKEN 64

t_class *sigenvrms_class;

typedef struct sigenvrms
{
  t_object x_obj; 	    	    /* header */
  void *x_outlet;		    /* a "float" outlet */
  void *x_clock;		    /* a "clock" object */
  t_sample *x_buf;		    /* a Hanning window */
  int x_phase;		    /* number of points since last output */
  int x_period;		    /* requested period of output */
  int x_realperiod;		    /* period rounded up to vecsize multiple */
  int x_npoints;		    /* analysis window size in samples */
  t_float x_result;		    /* result to output */
  t_sample x_sumbuf[MAXOVERLAP];	    /* summing buffer */
} t_sigenvrms;

static void sigenvrms_tick(t_sigenvrms *x);

static void *sigenvrms_new(t_floatarg fnpoints, t_floatarg fperiod)
{
  int npoints = fnpoints;
  int period = fperiod;
  t_sigenvrms *x;
  t_sample *buf;
  int i;

  if (npoints < 1) npoints = 1024;
  if (period < 1) period = npoints/2;
  if (period < npoints / MAXOVERLAP + 1)
    period = npoints / MAXOVERLAP + 1;
  if (!(buf = getbytes(sizeof(*buf) * (npoints + MAXVSTAKEN))))
    {
      error("env: couldn't allocate buffer");
      return (0);
    }
  x = (t_sigenvrms *)pd_new(sigenvrms_class);
  x->x_buf = buf;
  x->x_npoints = npoints;
  x->x_phase = 0;
  x->x_period = period;
  for (i = 0; i < MAXOVERLAP; i++) x->x_sumbuf[i] = 0;
  for (i = 0; i < npoints; i++)
    buf[i] = (1. - cos((2 * 3.141592654 * i) / npoints))/npoints;
  for (; i < npoints+MAXVSTAKEN; i++) buf[i] = 0;
  x->x_clock = clock_new(x, (t_method)sigenvrms_tick);
  x->x_outlet = outlet_new(&x->x_obj, gensym("float"));
  return (x);
}

static t_int *sigenvrms_perform(t_int *w)
{
  t_sigenvrms *x = (t_sigenvrms *)(w[1]);
  t_sample *in = (t_sample *)(w[2]);
  int n = (int)(w[3]);
  int count;
  t_sample *sump; 
  in += n;
  for (count = x->x_phase, sump = x->x_sumbuf;
       count < x->x_npoints; count += x->x_realperiod, sump++)
    {
      t_sample *hp = x->x_buf + count;
      t_sample *fp = in;
      t_sample sum = *sump;
      int i;
	
      for (i = 0; i < n; i++)
	{
          fp--;
          sum += *hp++ * (*fp * *fp);
	}
      *sump = sum;
    }
  sump[0] = 0;
  x->x_phase -= n;
  if (x->x_phase < 0)
    {
      x->x_result = x->x_sumbuf[0];
      for (count = x->x_realperiod, sump = x->x_sumbuf;
           count < x->x_npoints; count += x->x_realperiod, sump++)
        sump[0] = sump[1];
      sump[0] = 0;
      x->x_phase = x->x_realperiod - n;
      clock_delay(x->x_clock, 0L);
    }
  return (w+4);
}

static void sigenvrms_dsp(t_sigenvrms *x, t_signal **sp)
{
  if (x->x_period % sp[0]->s_n) x->x_realperiod =
    x->x_period + sp[0]->s_n - (x->x_period % sp[0]->s_n);
  else x->x_realperiod = x->x_period;
  dsp_add(sigenvrms_perform, 3, x, sp[0]->s_vec, sp[0]->s_n);
  if (sp[0]->s_n > MAXVSTAKEN) bug("sigenvrms_dsp");
}

static void sigenvrms_tick(t_sigenvrms *x)	/* callback function for the clock */
{
  outlet_float(x->x_outlet, sqrtf(x->x_result));
}

static void sigenvrms_ff(t_sigenvrms *x)		/* cleanup on free */
{
  clock_free(x->x_clock);
  freebytes(x->x_buf, (x->x_npoints + MAXVSTAKEN) * sizeof(*x->x_buf));
}

static void sigenvrms_help(void)
{
  post("envrms~\t:: envelope follower that does output rms instead of dB");
}


void envrms_tilde_setup(void)
{
  sigenvrms_class = class_new(gensym("envrms~"), (t_newmethod)sigenvrms_new,
                              (t_method)sigenvrms_ff, sizeof(t_sigenvrms), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
  class_addmethod(sigenvrms_class, nullfn, gensym("signal"), 0);
  class_addmethod(sigenvrms_class, (t_method)sigenvrms_dsp, gensym("dsp"), 0);

  class_addmethod(sigenvrms_class, (t_method)sigenvrms_help, gensym("help"), 0);
  zexy_register("envrms~");
}
