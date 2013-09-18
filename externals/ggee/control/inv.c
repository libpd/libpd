/* (C) Guenter Geiger <geiger@epy.co.at> */

#include <m_pd.h>


typedef struct inv
{
    t_object x_obj;
} t_inv;

static t_class *inv_class;

static void *inv_new(void)
{
    t_inv *x = (t_inv *)pd_new(inv_class);
    outlet_new(&x->x_obj, gensym("signal"));
    return (x);
}

static t_int *inv_perform(t_int *w)    /* not static; also used in d_fft.c */
{
    float *in = *(t_float **)(w+1), *out = *(t_float **)(w+2);
    t_int n = *(t_int *)(w+3);
    while (n--)
    {	
	*out++ = 1/ *in++;
    }
    return (w + 4);
}

static void inv_dsp(t_inv *x, t_signal **sp)
{
    dsp_add(inv_perform, 3, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void inv_tilde_setup(void)
{
    inv_class = class_new(gensym("inv~"), (t_newmethod)inv_new, 0,
    	sizeof(t_inv), 0, 0);

    class_addcreator(inv_new,gensym("1/x~"),0);
    
    class_addmethod(inv_class, nullfn, gensym("signal"), 0);
    class_addmethod(inv_class, (t_method)inv_dsp, gensym("dsp"), 0);
}




typedef struct scalarinv
{
    t_object x_obj;
} t_scalarinv;

static t_class *scalarinv_class;

static void *scalarinv_new(void)
{
    t_scalarinv *x = (t_scalarinv *)pd_new(scalarinv_class);
    outlet_new(&x->x_obj, gensym("float"));
    return (x);
}

static void scalarinv_float(t_scalarinv *x,t_float val)
{
     outlet_float(x->x_obj.ob_outlet,1.0f/val);
     
}

void inv_setup(void)
{
    scalarinv_class = class_new(gensym("inv"), (t_newmethod)scalarinv_new, 0,
    	sizeof(t_scalarinv), 0, 0);
    class_addcreator(scalarinv_new,gensym("1/x"),0);

    class_addfloat(scalarinv_class, (t_method)scalarinv_float);
}
