/* (C) Guenter Geiger <geiger@epy.co.at> */


/*

 These filter coefficients computations are taken from
 http://www.harmony-central.com/Computer/Programming/Audio-EQ-Cookbook.txt  

 written by Robert Bristow-Johnson

*/

#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif
#include <math.h>
#include "filters.h"



/* ------------------- equ ----------------------------*/
static t_class *equ_class;

void equ_bang(t_rbjfilter *x)
{
     t_atom at[5];
     t_float omega = e_omega(x->x_freq,x->x_rate);
     t_float alpha = e_alpha(x->x_bw*0.01,omega);
     t_float b0 = 1 + alpha*e_A(x->x_gain);
     t_float b1 = -2.*cos(omega);
     t_float b2 = 1 - alpha*e_A(x->x_gain);
     t_float a0 = 1 + alpha/e_A(x->x_gain);
     t_float a1 = -2.*cos(omega);
     t_float a2 = 1 - alpha/e_A(x->x_gain);

/*      post("bang %f %f %f",x->x_freq, x->x_gain, x->x_bw);*/
     
     if (!check_stability(-a1/a0,-a2/a0,b0/a0,b1/a0,b2/a0)) {
       post("equ: filter unstable -> resetting");
       a0=1.;a1=0.;a2=0.;
       b0=1.;b1=0.;b2=0.;
     }

     SETFLOAT(at,-a1/a0);
     SETFLOAT(at+1,-a2/a0);
     SETFLOAT(at+2,b0/a0);
     SETFLOAT(at+3,b1/a0);
     SETFLOAT(at+4,b2/a0);
     
     outlet_list(x->x_obj.ob_outlet,&s_list,5,at);
}


void equ_float(t_rbjfilter *x,t_floatarg f)
{
     x->x_freq = f;
     equ_bang(x);
}


static void *equ_new(t_floatarg f,t_floatarg g,t_floatarg bw)
{
    t_rbjfilter *x = (t_rbjfilter *)pd_new(equ_class);

    x->x_rate = 44100.0;
    outlet_new(&x->x_obj,&s_float);
    floatinlet_new(&x->x_obj, &x->x_gain);    
    floatinlet_new(&x->x_obj, &x->x_bw);
    if (f > 0.) x->x_freq = f;
    if (bw > 0.) x->x_bw = bw;
    if (g != 0.) x->x_gain = g;
    return (x);
}


void equalizer_setup(void)
{
    equ_class = class_new(gensym("equalizer"), (t_newmethod)equ_new, 0,
				sizeof(t_rbjfilter), 0,A_DEFFLOAT,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addbang(equ_class,equ_bang);
    class_addfloat(equ_class,equ_float);
}





