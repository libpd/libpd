/* (C) Guenter Geiger <geiger@epy.co.at> */


#include <m_pd.h>
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ shuffle ----------------------------- */

static t_class *shuffle_class;


typedef struct _shuffle
{
     t_object x_obj;
     t_float x;
} t_shuffle;


void shuffle_float(t_shuffle *x, t_floatarg f)
{
     post("float %f",f);
     x->x = f;
}



static t_int *shuffle_perform(t_int *w)
{
     t_shuffle* x = (t_shuffle*)(w[1]);
     t_float* in1 = (t_float*) w[2];
     t_float* in2 = (t_float*) w[3];
     t_float* out = (t_float*) w[4];
     int n = w[5];

     if (x->x <= 0) {
	  while (n--) {
	       *out++ = *in1++;
	  }    
	  return w+6;
     }

     if (x->x < 0.5) {
	  t_int index = 1/x->x;
	  while (n--) {
	       if (n%index){ 
		    *out++ = *in1++;
		    in2++;
	       }
	       else {
		    *out++ = *in2++;
		    in1++;
	       }
	  }
	  return w+6;
     }

     if (x->x > 1.0) {
	  while (n--) {
	       *out++ = *in2++;
	  }    
	  return w+6;
     }

     if (x->x >= 0.5) {
	  t_int index = 1/(1.0- x->x);
	  while (n--) {
	       if (n%index) { 
		    *out++ = *in2++;
		    in1++;
	       }
	       else {
		    *out++ = *in1++;
		    in2++;
	       }
	  }
     }

     return w+6;
}


static void shuffle_dsp(t_shuffle *x, t_signal **sp)
{
	  dsp_add(shuffle_perform, 5, x, sp[0]->s_vec, 
		  sp[1]->s_vec,sp[2]->s_vec, sp[0]->s_n);

}

static void *shuffle_new()
{
    t_shuffle *x = (t_shuffle *)pd_new(shuffle_class);

    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);


    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

void shuffle_setup(void)
{
    shuffle_class = class_new(gensym("shuffle~"), (t_newmethod)shuffle_new, 0,
				sizeof(t_shuffle), 0,0);
    
    class_addmethod(shuffle_class, nullfn, gensym("signal"), 0);
    class_addmethod(shuffle_class, (t_method) shuffle_dsp, gensym("dsp"), 0);
    
    class_addfloat(shuffle_class,shuffle_float);
}


