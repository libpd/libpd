/*
 *   filterortho.cc  -  orthogonal biquad filter pd interface 
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



typedef struct filterortho_struct
{
    t_object x_obj;
    t_float x_f;
    DSPIfilterOrtho *filterortho;
} t_filterortho;

void filterortho_bang(t_filterortho *x)
{

}


static t_int *filterortho_perform(t_int *w)
{


  t_float *in    = (t_float *)(w[3]);
  t_float *out    = (t_float *)(w[4]);
  DSPIfilterOrtho* filterortho  = (DSPIfilterOrtho *)(w[1]);
  t_int n = (t_int)(w[2]);
  t_int i;
  t_float x;


  // dit kan beter 
  t_float smooth = 1.0 - pow(.05,1.0/(t_float)(n));

  for (i = 0; i < n; i++)
  {
      x  = *in++;
      filterortho->BangSmooth(x, x, smooth);
      *out++ = x;
  }

  filterortho->killDenormals();

  return (w+5);
}

static void filterortho_dsp(t_filterortho *x, t_signal **sp)
{
    dsp_add(filterortho_perform, 4, x->filterortho, sp[0]->s_n, sp[0]->s_vec, sp[1]->s_vec);

}                                  
void filterortho_free(t_filterortho *x)
{
    delete x->filterortho;

}

t_class *filterortho_class;



void setLP(t_filterortho *x,  t_floatarg f, t_floatarg Q) {x->filterortho->setLP(f / sys_getsr(), Q);}
void setHP(t_filterortho *x,  t_floatarg f, t_floatarg Q) {x->filterortho->setHP(f / sys_getsr(), Q);}
void setBP(t_filterortho *x,  t_floatarg f, t_floatarg Q) {x->filterortho->setBP(f / sys_getsr(), Q);}
void setBR(t_filterortho *x,  t_floatarg f, t_floatarg Q) {x->filterortho->setBR(f / sys_getsr(), Q);}
void setAP(t_filterortho *x,  t_floatarg f, t_floatarg Q) {x->filterortho->setAP(f / sys_getsr(), Q);}

void setLS(t_filterortho *x,  t_floatarg f, t_floatarg A) {x->filterortho->setLS(f / sys_getsr(), A);}
void setHS(t_filterortho *x,  t_floatarg f, t_floatarg A) {x->filterortho->setHS(f / sys_getsr(), A);}

void setEQ(t_filterortho *x,  t_floatarg f, t_floatarg Q, t_floatarg A) {x->filterortho->setEQ(f / sys_getsr(), Q, A);}


void *filterortho_new()
{
    t_filterortho *x = (t_filterortho *)pd_new(filterortho_class);
    x->filterortho = new DSPIfilterOrtho();
    outlet_new(&x->x_obj, gensym("signal")); 
    setLP(x, 10000, 2);
    return (void *)x;
}



extern "C" {

void filterortho_tilde_setup(void)
{
    //post("filterortho~ v0.1");
    filterortho_class = class_new(gensym("filterortho~"), (t_newmethod)filterortho_new,
    	(t_method)filterortho_free, sizeof(t_filterortho), 0, A_NULL);

    CLASS_MAINSIGNALIN(filterortho_class, t_filterortho, x_f); 


    class_addmethod(filterortho_class, (t_method)filterortho_bang, gensym("bang"), A_NULL);

    class_addmethod(filterortho_class, (t_method)filterortho_dsp, gensym("dsp"), A_NULL); 

    class_addmethod(filterortho_class, (t_method)setLP, gensym("setLP"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setHP, gensym("setHP"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setBP, gensym("setBP"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setBR, gensym("setBR"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setAP, gensym("setAP"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setLS, gensym("setLS"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setHS, gensym("setHS"), A_FLOAT, A_FLOAT, A_NULL); 
    class_addmethod(filterortho_class, (t_method)setEQ, gensym("setEQ"), A_FLOAT, A_FLOAT, A_FLOAT, A_NULL); 
 
}

}
