/*
 *   biquadseries.cc  -  second order section filter pd interface 
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

#include "DSPIcomplex.h"
#include "DSPIfilters.h"



typedef struct biquadseries_struct
{
    t_object x_obj;
    t_float x_f;
    DSPIfilterSeries* biquadseries;
} t_biquadseries;

void biquadseries_bang(t_biquadseries *x)
{

}

void biquadseries_butterLP(t_biquadseries *x,  t_floatarg f)
{
    x->biquadseries->setButterLP(f / sys_getsr());
}

void biquadseries_butterHP(t_biquadseries *x,  t_floatarg f)
{
    x->biquadseries->setButterHP(f / sys_getsr());
}



static t_int *biquadseries_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  DSPIfilterSeries* biquadseries  = (DSPIfilterSeries *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_int i;
  t_float x;

  // dit kan beter 
  t_float smooth = .01;
  //1.0f - pow(.9f,1.0f/(float)(n));

  for (i = 0; i < n; i++)
  {
      x  = *in++;
      biquadseries->BangSmooth(x, x, smooth);
      *out++ = x;
  }

  return (w+5);
}

static void biquadseries_dsp(t_biquadseries *x, t_signal **sp)
{
    dsp_add(biquadseries_perform, 4, x->biquadseries, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);

}                                  
void biquadseries_free(void)
{

}

t_class *biquadseries_class;

void *biquadseries_new(t_floatarg fsections)
{
    t_biquadseries *x = (t_biquadseries *)pd_new(biquadseries_class);

    int sections = (int)fsections;
    if (sections < 1) sections = 1;
    // post("biquadseries~: %d sections", sections);
    x->biquadseries = new DSPIfilterSeries(sections);

    //    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("freq"));  
    outlet_new(&x->x_obj, gensym("signal")); 

    biquadseries_butterLP(x, 10000);

    return (void *)x;
}


extern "C" {

void biquadseries_tilde_setup(void)
{
    //post("biquadseries~ v0.1");

    biquadseries_class = class_new(gensym("biquadseries~"), (t_newmethod)biquadseries_new,
    	(t_method)biquadseries_free, sizeof(t_biquadseries), 0, A_DEFFLOAT, 0);

    CLASS_MAINSIGNALIN(biquadseries_class, t_biquadseries, x_f); 

    class_addmethod(biquadseries_class, (t_method)biquadseries_bang, gensym("bang"), (t_atomtype)0);

    class_addmethod(biquadseries_class, (t_method)biquadseries_dsp, gensym("dsp"), (t_atomtype)0); 

    class_addmethod(biquadseries_class, (t_method)biquadseries_butterLP, gensym("butterLP"), A_FLOAT, A_NULL); 
    class_addmethod(biquadseries_class, (t_method)biquadseries_butterHP, gensym("butterHP"), A_FLOAT, A_NULL); 

}

}
