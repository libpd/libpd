/*
 *   dynwav~.c  - dynamic wavetable oscillators
 *
 *   Copyright (c) 2000-2003 by Tom Schouten
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "m_pd.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#define MAXORDER 1024

typedef struct dynwavctl
{
  t_float *c_buf1; /* current */
  t_float *c_buf2; /* old */
  t_int c_order;

} t_dynwavctl;

typedef struct dynwav
{
  t_object x_obj;
  t_float x_f;
  t_dynwavctl x_ctl;
} t_dynwav;


static t_int *dynwav_perform(t_int *w)
{


  t_float *wave     = (t_float *)(w[3]);
  t_float *freq     = (t_float *)(w[4]);
  t_float *out      = (t_float *)(w[5]);
  t_dynwavctl *ctl  = (t_dynwavctl *)(w[1]);
  t_int n           = (t_int)(w[2]);

  t_float *buf, *dbuf, *swap;

  int i;
  int mask = n-1;


  /* swap buffer pointers */
  swap = ctl->c_buf1;               /* this is the last one stored */
  buf  = ctl->c_buf1 = ctl->c_buf2; /* put oldest in newest to overwrite */
  dbuf = ctl->c_buf2 = swap;        /* put last one in oldest */
  

  if (buf && dbuf)
    {

      /* store input wavetable in buffer */
      memcpy(buf, wave, n*sizeof(t_float));
      
      
      for (i = 0; i < n; i++)
	{
	  t_float findex = *freq++ * (t_float)n;
	  int index = findex;
	  t_float frac,  a,  b,  c,  d, cminusb, q, r;
	  int ia, ib, ic, id;
	  
	  frac = findex - index; 

	  ia = (index-1) & mask;
	  ib = (index  ) & mask;
	  ic = (index+1) & mask;
	  id = (index+2) & mask;

	  q = i+1;
	  q /= n;

	  r = n-1-i;
	  r /= n;
	  
	  /* get 4 points, wrap index */
	  a = q * buf[ia] + r * dbuf[ia];
	  b = q * buf[ib] + r * dbuf[ib];
	  c = q * buf[ic] + r * dbuf[ic];
	  d = q * buf[id] + r * dbuf[id];
	  
	  cminusb = c-b;
	  *out++ = b + frac * (cminusb - 0.5f * (frac-1.) * 
			       ((a - d + 3.0f * cminusb) * frac + 
				(b - a - cminusb)));
	}    
      
    }
  return (w+6);
}

static t_int *dynwav_perform_8point(t_int *w) 
/* FIXME: i thought this was broken. */
{


  t_float *wave     = (t_float *)(w[3]);
  t_float *freq     = (t_float *)(w[4]);
  t_float *out      = (t_float *)(w[5]);
  t_dynwavctl *ctl  = (t_dynwavctl *)(w[1]);
  t_int n           = (t_int)(w[2]);

  t_float *buf, *dbuf, *swap;

  int i;
  int mask = n-1;


  /* swap buffer pointers */
  swap = ctl->c_buf1;               /* this is the last one stored */
  buf  = ctl->c_buf1 = ctl->c_buf2; /* put oldest in newest to overwrite */
  dbuf = ctl->c_buf2 = swap;        /* put last one in oldest */
  

  if (buf && dbuf)
    {

/* const t_float N1 = 1 / (   2      * (1-(1/9))  * (1-(1/25))  * (1-(1/49))  );
** const t_float N2 = 1 / ( (1-(9))  *      2     * (1-(9/25))  * (1-(9/49))  );
** const t_float N3 = 1 / ( (1-(25)) * (1-(25/9)) *      2      * (1-(25/49)) );
** const t_float N4 = 1 / ( (1-(49)) * (1-(49/9)) * (1-(49/25)) *    2        );
*/

      const t_float N1 =   0.59814453125;
      const t_float N2 =  -0.11962890625;
      const t_float N3 =   0.02392578125;
      const t_float N4 =  -0.00244140625;


      /* store input wavetable in buffer */
      memcpy(buf, wave, n*sizeof(t_float));
      
      
      for (i = 0; i < n; i++)
	{
	  t_float findex = *freq++ * (t_float)n;
	  int index = findex;
	  t_float frac, q, r, fm, fp, fe, fo;
	  t_float x1, x2, x3, x4;
	  t_float g1, g2, g3, g4;
	  t_float gg, g2g3g4, g1g3g4, g1g2g4, g1g2g3;
	  t_float acc;
	  int im, ip;
	  
	  frac = 2 *(findex - index) - 1; 

	  x1 = frac;
	  x2 = frac/3;
	  x3 = frac/5;
	  x4 = frac/7;

	  g1 = 1 - x1*x1;
	  g2 = 1 - x2*x2;
	  g3 = 1 - x3*x3;
	  g4 = 1 - x4*x4;

	  gg       = g3 * g4;
	  g2g3g4   = g2 * gg;       /* 1 */
	  g1g3g4   = g1 * gg;       /* 2 */
	  gg       = g1 * g2;
	  g1g2g4   = g4 * gg;       /* 3 */
	  g1g2g3   = g3 * gg;       /* 4 */


	  /* triangle interpolation between current and past wavetable*/
	  q = i+1;
	  q /= n;

	  r = n-1-i;
	  r /= n;


	  /* 1, -1*/
	  im =  (index  ) & mask;
	  ip =  (index+1) & mask;
	  fm =  q * buf[im] + r * dbuf[im];
	  fp =  q * buf[ip] + r * dbuf[ip];
	  fe = fp + fm;
	  fo = fp - fm;

	  acc = N1 * g2g3g4 * (fe + x1*fo);

	  /* 2, -2 */
	  im =  (index-1) & mask;
	  ip =  (index+2) & mask;
	  fm =  q * buf[im] + r * dbuf[im];
	  fp =  q * buf[ip] + r * dbuf[ip];
	  fe = fp + fm;
	  fo = fp - fm;

	  acc += N2 * g1g3g4 * (fe + x2*fo);

	  /* 3, -3 */
	  im =  (index-2) & mask;
	  ip =  (index+3) & mask;
	  fm =  q * buf[im] + r * dbuf[im];
	  fp =  q * buf[ip] + r * dbuf[ip];
	  fe = fp + fm;
	  fo = fp - fm;

	  acc += N3 * g1g2g4 * (fe + x3*fo);

	  /* 4, -4 */
	  im =  (index-3) & mask;
	  ip =  (index+4) & mask;
	  fm =  q * buf[im] + r * dbuf[im];
	  fp =  q * buf[ip] + r * dbuf[ip];
	  fe = fp + fm;
	  fo = fp - fm;

	  acc += N4 * g1g2g3 * (fe + x4*fo);


	  *out++ = acc;

	}    
      
    }
  return (w+6);
}


static void dynwav_dsp(t_dynwav *x, t_signal **sp)
{
  int n = sp[0]->s_n;
  int k;

  if (x->x_ctl.c_order != n)
    {
      if (x->x_ctl.c_buf1) free (x->x_ctl.c_buf1);
      if (x->x_ctl.c_buf2) free (x->x_ctl.c_buf2);

      x->x_ctl.c_buf1 = (t_float *)malloc(n*sizeof(t_float));
      x->x_ctl.c_buf2 = (t_float *)malloc(n*sizeof(t_float));

      for(k=0; k<n; k++)
	{
	  x->x_ctl.c_buf1[k] = 0;
	  x->x_ctl.c_buf2[k] = 0;
	}

      x->x_ctl.c_order = n;
    }


  dsp_add(dynwav_perform_8point, 5, &x->x_ctl, sp[0]->s_n, 
	  sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec);


}                                  
static void dynwav_free(t_dynwav *x)
{

  if (x->x_ctl.c_buf1) free (x->x_ctl.c_buf1);
  if (x->x_ctl.c_buf2) free (x->x_ctl.c_buf2);

}

t_class *dynwav_class;

static void *dynwav_new(t_floatarg order)
{
    t_dynwav *x = (t_dynwav *)pd_new(dynwav_class);
    int iorder = (int)order;
    int i, n=64, k;

    /* in 2 */
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("signal"), gensym("signal"));  

    /* out 1 */
    outlet_new(&x->x_obj, gensym("signal")); 



    /* init data */
    x->x_ctl.c_buf1 = (t_float *)malloc(n*sizeof(t_float));
    x->x_ctl.c_buf2 = (t_float *)malloc(n*sizeof(t_float));
    
    for(k=0; k<n; k++)
      {
	x->x_ctl.c_buf1[k] = 0;
	x->x_ctl.c_buf2[k] = 0;
      }
    
    x->x_ctl.c_order = n;

    return (void *)x;
}

void dynwav_tilde_setup(void)
{
  //post("dynwav~ v0.1");
    dynwav_class = class_new(gensym("dynwav~"), (t_newmethod)dynwav_new,
    	(t_method)dynwav_free, sizeof(t_dynwav), 0, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(dynwav_class, t_dynwav, x_f);
    class_addmethod(dynwav_class, (t_method)dynwav_dsp, gensym("dsp"), 0); 


}

