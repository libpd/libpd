
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

/* ------------------- bandpass ----------------------------*/

static t_class *bandpass_class;

void bandpass_bang(t_rbjfilter *x)
{
     t_atom at[5];
     t_float omega = e_omega(x->x_freq,x->x_rate);
     t_float alpha = e_alpha(x->x_bw* 0.01,omega);
     t_float b1 = 0.;
     t_float b0 = alpha;
     t_float b2 = -alpha;
     t_float a0 = 1 + alpha;
     t_float a1 = -2.*cos(omega);
     t_float a2 = 1 - alpha;

/*     post("bang %f %f %f",x->x_freq, x->x_gain, x->x_bw); */
     
     if (!check_stability(-a1/a0,-a2/a0,b0/a0,b1/a0,b2/a0)) {
       post("bandpass: filter unstable -> resetting");
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


void bandpass_float(t_rbjfilter *x,t_floatarg f)
{
     x->x_freq = f;
     bandpass_bang(x);
}


static void *bandpass_new(t_floatarg f,t_floatarg bw)
{
    t_rbjfilter *x = (t_rbjfilter *)pd_new(bandpass_class);

    x->x_rate = 44100.0;
    outlet_new(&x->x_obj,&s_float);
/*    floatinlet_new(&x->x_obj, &x->x_gain);    */
    floatinlet_new(&x->x_obj, &x->x_bw);
    if (f > 0.) x->x_freq = f;
    if (bw > 0.) x->x_bw = bw;
    return (x);
}


void bandpass_setup(void)
{
    bandpass_class = class_new(gensym("bandpass"), (t_newmethod)bandpass_new, 0,
				sizeof(t_rbjfilter), 0,A_DEFFLOAT,A_DEFFLOAT,0);
    class_addbang(bandpass_class,bandpass_bang);
    class_addfloat(bandpass_class,bandpass_float);
}



