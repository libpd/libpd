/* Copyright (c) 1997-1999 Miller Puckette source modified by dieb13.
* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/*  catch13~ ...  = original objects with set-message for changing - bad hack - but works somehow*/

#include "m_pd.h"
#include "d_global13.h"

#define DEFSENDVS 64	/* LATER get send to get this from canvas */

/* ----------------------------- send13~ ----------------------------- */

 void *sigsend13_new(t_symbol *s)
{
    t_sigsend13 *x = (t_sigsend13 *)pd_new(sigsend13_class);
    if (!*s->s_name) s = gensym("send13~");
    pd_bind(&x->x_obj.ob_pd, s);
    x->x_sym = s;
    x->x_n = DEFSENDVS;
    x->x_vec = (float *)getbytes(DEFSENDVS * sizeof(float));
    return (x);
}

 t_int *sigsend13_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *out = (t_float *)(w[2]);
    int n = (int)(w[3]);
    while (n--)
    {
	/* TB: denormal handling in pd >0.37-2 */
#ifdef PD_BIGORSMALL
	*out = (PD_BIGORSMALL(*in) ? 0 : *in);
	out++;
	in++;
#else
	*out++ = *in++;
#endif
    }; 
    return (w+4);
}

 void sigsend13_set(t_sigsend13 *x, t_symbol *s)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
    x->x_sym = s;
    pd_bind(&x->x_obj.ob_pd, s);
}


 void sigsend13_dsp(t_sigsend13 *x, t_signal **sp)
{
    if (x->x_n == sp[0]->s_n)
    	dsp_add(sigsend13_perform, 3, sp[0]->s_vec, x->x_vec, sp[0]->s_n);
    else error("sigsend13 %s: unexpected vector size", x->x_sym->s_name);
}

 void sigsend13_free(t_sigsend13 *x)
{
    pd_unbind(&x->x_obj.ob_pd, x->x_sym);
    freebytes(x->x_vec, x->x_n * sizeof(float));
}

void sigsend13_setup(void)
{
    sigsend13_class = class_new(gensym("send13~"), (t_newmethod)sigsend13_new,
    	(t_method)sigsend13_free, sizeof(t_sigsend13), 0, A_DEFSYM, 0);
    class_addmethod(sigsend13_class, (t_method)sigsend13_set, gensym("set"),A_SYMBOL, 0); 	
    
    
    class_addcreator((t_newmethod)sigsend13_new, gensym("s13~"), A_DEFSYM, 0);
    class_addmethod(sigsend13_class, nullfn, gensym("signal"), 0);
    class_addmethod(sigsend13_class, (t_method)sigsend13_dsp, gensym("dsp"), 0);
}

void send13_tilde_setup()
{
  sigsend13_setup();
}
