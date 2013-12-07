/* spigot~ -- route a signal to its right or left output  */

/*  Copyleft 2001 Yves Degoyon.
Permission is granted to use this software for any purpose provided you
keep this copyright notice intact.

THE AUTHOR AND HIS EXPLOITERS MAKE NO WARRANTY, EXPRESS OR IMPLIED,
IN CONNECTION WITH THIS SOFTWARE.

*/

#include "m_pd.h"

typedef struct _spigot
{
    t_object x_obj;
    t_int x_on;
    t_float x_f;
} t_spigot;

static t_class *spigot_class;

static char   *spigot_version = "spigot~: a signal router : version 0.1, written by Yves Degoyon (ydegoyon@free.fr)";

static void *spigot_new(void)
{
    t_spigot *x = (t_spigot *)pd_new(spigot_class);
    x->x_on = 0;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("seton") );
    outlet_new(&x->x_obj, &s_signal);
    outlet_new(&x->x_obj, &s_signal);
    return (x);
}

static t_int *spigot_perform(t_int *w)
{
    t_float *in = (t_float *)(w[1]);
    t_float *outr = (t_float *)(w[2]);
    t_float *outl = (t_float *)(w[3]);
    int n = (int)(w[4]);
    t_spigot* x = (t_spigot*)(w[5]);
    while (n--)
    {
        if ( (x->x_on)==0.0 )
        {
            *(outl)=0.0;
            *(outr)=*(in);
        }
        else
        {
            *(outl)=*(in);
            *(outr)=0.0;
        }
        in++;
        outl++;
        outr++;
    }
    return (w+6);
}

static void spigot_dsp(t_spigot *x, t_signal **sp)
{
    dsp_add(spigot_perform, 5, sp[0]->s_vec, sp[1]->s_vec, sp[2]->s_vec,
            sp[0]->s_n, x);
}

static void spigot_set(t_spigot *x, t_float f)
{
    x->x_on=f;
}

void spigot_tilde_setup(void)
{
    logpost(NULL, 4, spigot_version );
    spigot_class = class_new(gensym("spigot~"), (t_newmethod)spigot_new, 0,
                             sizeof(t_spigot), 0, 0);
    CLASS_MAINSIGNALIN( spigot_class, t_spigot, x_f );
    class_addmethod(spigot_class, (t_method)spigot_dsp, gensym("dsp"), 0);
    class_addmethod(spigot_class, (t_method)spigot_set, gensym("seton"), A_FLOAT, 0);
}
