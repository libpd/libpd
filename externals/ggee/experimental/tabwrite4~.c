
#include <m_pd.h>
#include <math.h>
#include <stdio.h> // fprintf

/* ------------------------- tabwrite4~ -------------------------- */

static t_class *tabwrite4_tilde_class;

typedef struct _tabwrite4_tilde
{
    t_object x_obj;
    int x_phase;
    int x_npoints;
    float *x_vec;
    t_symbol *x_arrayname;
    float x_f;
    t_sample x_1;
    t_sample x_2;
    t_sample x_3;
    t_sample x_4;
    float x_index;
} t_tabwrite4_tilde;

static void tabwrite4_tilde_tick(t_tabwrite4_tilde *x);

static void *tabwrite4_tilde_new(t_symbol *s)
{
    t_tabwrite4_tilde *x = (t_tabwrite4_tilde *)pd_new(tabwrite4_tilde_class);
    x->x_phase = 0x7fffffff;
    x->x_arrayname = s;
    x->x_f = 0;
    x->x_1 = 0.;
    x->x_2 = 0.;
    x->x_3 = 0.;
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_signal, &s_signal);
    outlet_new(&x->x_obj, &s_signal);       
    return (x);
}

static void tabwrite4_tilde_redraw(t_tabwrite4_tilde *x)
{
    t_garray *a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class);
    if (!a)
        bug("tabwrite4_tilde_redraw");
    else garray_redraw(a);
}

static t_int *tabwrite4_tilde_perform(t_int *w)
{
    t_tabwrite4_tilde *x = (t_tabwrite4_tilde *)(w[1]);
    t_float *in1 = (t_float *)(w[2]);
    t_float *in2 = (t_float *)(w[3]);
    int n = (int)(w[4]);    
    t_float* end = in1 + n;
    t_float* end2 = in2 + n;

    float *buf = x->x_vec, *fp, a,b,c,d;
    float findex = *in2;
    float frac;
    int iindex = (int)findex;
    int wraparound = 0;
    int maxindex = x->x_npoints-1;

    if (!buf) return (w+5);

    b = x->x_1;
    c = x->x_2;
    d = x->x_3;

    while (n--)
    {
        float cminusb;
        findex = *in2++;

#if 0
        while ((int)findex > iindex) {
            iindex++;
            *(buf+iindex) = *(buf+iindex-1);
        }
#endif

        iindex = (int)findex;

//        post("iindex %d, findex %f",iindex,findex);
        if (in2 < end2 && findex > *in2) {
            wraparound = 1;
            post("wraparound");
        }


        a=b;
        b=c;
        c=d;
        d = *in1++;

        frac = findex - iindex;

        iindex +=64; // one blocksize and 4 points
        if (iindex < 0) {
            iindex += maxindex+1;
        }
        else if (iindex > maxindex) {
            iindex -= maxindex;          
        }

        fp = buf + iindex;
        cminusb = c-b;

        *fp = b + frac * (
                    cminusb - 0.1666667f * (1.-frac) * (
                    (d - a - 3.0f * cminusb) * frac + (d + 2.0f*a - 3.0f*b)
                    )
                    );

    }
    post("written to %d",iindex);
    x->x_1 = b;
    x->x_2 = c;
    x->x_3 = d;

#if 0
    buf[maxindex-2] = buf[maxindex-3]*0.5; 
    buf[maxindex-1] = buf[maxindex-2]*0.5; 
    buf[maxindex] = buf[maxindex-1]*0.5; 
    buf[2] = buf[3]*0.5; 
    buf[1] = buf[2]*0.5; 
    buf[0] = buf[1]*0.5; 
#endif

    if (wraparound)  tabwrite4_tilde_redraw(x);    

    return (w+5);
}


void tabwrite4_tilde_set(t_tabwrite4_tilde *x, t_symbol *s)
{
    t_garray *a;

    x->x_arrayname = s;
    if (!(a = (t_garray *)pd_findbyclass(x->x_arrayname, garray_class)))
    {
        if (*s->s_name) pd_error(x, "tabwrite4~: %s: no such array",
            x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else if (!garray_getfloatarray(a, &x->x_npoints, &x->x_vec))
    {
        pd_error(x, "%s: bad template for tabwrite4~", x->x_arrayname->s_name);
        x->x_vec = 0;
    }
    else garray_usedindsp(a);
}

static void tabwrite4_tilde_dsp(t_tabwrite4_tilde *x, t_signal **sp)
{
    tabwrite4_tilde_set(x, x->x_arrayname);
    dsp_add(tabwrite4_tilde_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void tabwrite4_tilde_bang(t_tabwrite4_tilde *x)
{
    x->x_phase = 0;
}

static void tabwrite4_tilde_start(t_tabwrite4_tilde *x, t_floatarg f)
{
    x->x_phase = (f > 0 ? f : 0);
}

static void tabwrite4_tilde_stop(t_tabwrite4_tilde *x)
{
    if (x->x_phase != 0x7fffffff)
    {
        tabwrite4_tilde_redraw(x);
        x->x_phase = 0x7fffffff;
    }
}

void tabwrite4_tilde_setup(void)
{
    tabwrite4_tilde_class = class_new(gensym("tabwrite4~"),
        (t_newmethod)tabwrite4_tilde_new, 0,
        sizeof(t_tabwrite4_tilde), 0, A_DEFSYM, 0);
    CLASS_MAINSIGNALIN(tabwrite4_tilde_class, t_tabwrite4_tilde, x_f);
    class_addmethod(tabwrite4_tilde_class, (t_method)tabwrite4_tilde_dsp,
        gensym("dsp"), 0);
    class_addmethod(tabwrite4_tilde_class, (t_method)tabwrite4_tilde_set,
        gensym("set"), A_SYMBOL, 0);
    class_addmethod(tabwrite4_tilde_class, (t_method)tabwrite4_tilde_stop,
        gensym("stop"), 0);
    class_addmethod(tabwrite4_tilde_class, (t_method)tabwrite4_tilde_start,
        gensym("start"), A_DEFFLOAT, 0);
    class_addbang(tabwrite4_tilde_class, tabwrite4_tilde_bang);
}



