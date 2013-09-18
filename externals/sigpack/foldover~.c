/* sIgpAck
 * for
 * pure-data
 * www.weiss-archiv.de */

#include "m_pd.h"
#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

/* ------------------------ foldover~ ----------------------------- */
/* foldover distortion */
/* code from swh_plugins by steve harris www.plugin.org.uk */

static t_class *foldover_tilde_class;

typedef struct _foldover_tilde
{
    t_object x_obj;
	t_sample x_drive_p;
	t_sample x_push;
	float x_f;
} t_foldover_tilde;

static void *foldover_tilde_new(t_floatarg drive_p, t_floatarg push)
{
    t_foldover_tilde *x = (t_foldover_tilde *)pd_new(foldover_tilde_class);
	x->x_drive_p = drive_p;
	x->x_push = push;
    outlet_new(&x->x_obj, gensym("signal"));
	floatinlet_new(&x->x_obj, &x->x_drive_p);
	floatinlet_new(&x->x_obj, &x->x_push);
	x->x_f = 0;
	if(drive_p) x->x_drive_p = drive_p;
	else x->x_drive_p = 0;
	if(push) x->x_push = push;
	else x->x_push = 0;
    return (x);
}

static t_int *foldover_tilde_perform(t_int *w)
{
	t_foldover_tilde *x = (t_foldover_tilde *)(w[1]);
    t_float *in = (t_float *)(w[2]);
    t_float *out = (t_float *)(w[3]);
    int n = (int)(w[4]);
	float f, value, y;
	float drive = x->x_drive_p + 1.0f;
    while (n--)
    {
		f = *in++;
		y = f * drive + x->x_push;
		value = 1.5f * y - 0.5f * y * y *y;
		*out++ = value;
    }
    return (w+5);
}

static void foldover_tilde_dsp(t_foldover_tilde *x, t_signal **sp)
{
    dsp_add(foldover_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

void foldover_tilde_setup(void)
{
    foldover_tilde_class = class_new(gensym("foldover~"), (t_newmethod)foldover_tilde_new, 0,
    	sizeof(t_foldover_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
    CLASS_MAINSIGNALIN(foldover_tilde_class, t_foldover_tilde, x_f);
    class_addmethod(foldover_tilde_class, (t_method)foldover_tilde_dsp, gensym("dsp"), 0);
}
