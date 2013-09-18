/* runningmean.c MP 20080516 */
/* output the running mean of the input */
#include "m_pd.h"

/* We implement a circular buffer x_data of length x_n */
/* With each incoming value we sum the x_n values, then divide by */
/* x_n to get the mean value, x_mean. */

#define RUNNINGMEAN_MAX 128 /* a default value when no valid argument is supplied */

typedef struct _runningmean
{
    t_object        x_obj;
    t_int           x_in1;
    t_int           x_in2;
    t_int           x_in3;
    t_outlet        *x_out;
    t_inlet         *x_inlet2;
    t_int           x_n;
    t_int           x_originalsize;
    t_float         *x_data;
    t_float         x_mean;
    t_int           x_pointer;
} t_runningmean;

static t_class *runningmean_class;

void runningmean_setup(void);
static void *runningmean_new(t_floatarg f);
static void runningmean_free(t_runningmean *x);
static void runningmean_bang(t_runningmean *x);
static void runningmean_float(t_runningmean *x, t_float f);
static void runningmean_length(t_runningmean *x, t_float f);
static void runningmean_zero(t_runningmean *x);

static void runningmean_float(t_runningmean *x, t_float f)
{
    float   *p = x->x_data;
    float   total = 0;
    int     i;

    if (x->x_n > 0)
    {
        /* add a float at the current location, overwriting the oldest data */
        x->x_data[x->x_pointer] = f;
        if (++x->x_pointer >= x->x_n) x->x_pointer = 0; /* wrap pointer */
        for (i = 0; i < x->x_n; ++i) total += *p++;
        x->x_mean = total/x->x_n;
        outlet_float(x->x_out, x->x_mean);
    }
    return;
}

static void runningmean_bang(t_runningmean *x)
{
    outlet_float(x->x_out, x->x_mean);
    return;
}

static void runningmean_length(t_runningmean *x, t_float f)
{

    if ((f >= 1) && ((int)f == f) && (f <= x->x_originalsize))
    {
        x->x_n = (int)f;
        runningmean_zero(x);
    }
    else post("runningmean length must be an integer between 1 and %d.", x->x_originalsize);
    return;
}

static void runningmean_zero(t_runningmean *x)
{
    float   *p = x->x_data;
    int     i;

    /* zero the entire array */
    for (i = 0; i < x->x_n; ++i) *p++ = 0;
    x->x_mean = 0;
    x->x_pointer = 0;
    return;
}


static void runningmean_free(t_runningmean *x)
{
    freebytes(x->x_data, x->x_originalsize);
    x->x_originalsize = x->x_n = 0;
    x->x_data = NULL;
    return;
}

static void *runningmean_new(t_floatarg f)
{
    t_runningmean   *x;

    x = (t_runningmean *)pd_new(runningmean_class);
    if (x == NULL) return (x);
    x->x_out = outlet_new((t_object *)x, &s_float);
    x->x_inlet2 = inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("length"));
    if (!((f >= 1) && ((int)f == f)))
    {
        post("runningmean length %0.2f must be an integer greater than 1, using %d", f, RUNNINGMEAN_MAX);
        f = RUNNINGMEAN_MAX;
    }
    {
        x->x_n = (int)f;
        x->x_data = (t_float *)getbytes(sizeof(float)*x->x_n);
        if (x->x_data == NULL)
        {
            post("runningmean unable to allocate %lu bytes of memory, using %d", sizeof(float)*x->x_n, RUNNINGMEAN_MAX);
            x->x_n = RUNNINGMEAN_MAX;
            //x->x_data = (t_float *)getbytes(x->x_n);
            if (x->x_data == NULL)
            {
                post("runningmean unable to allocate %lu bytes of memory, using 0", x->x_n);
                x->x_n = 0;
            }
        }
        x->x_originalsize = x->x_n;
        runningmean_zero(x);
    }
    return (x);
}

void runningmean_setup(void)
{
    runningmean_class = class_new
    (
        gensym("runningmean"),
        (t_newmethod)runningmean_new,
        (t_method)runningmean_free,
        sizeof(t_runningmean),
        CLASS_DEFAULT,
        A_DEFFLOAT,
        0
    ); /* one argument for length */
    class_addbang(runningmean_class, runningmean_bang);
    class_addfloat(runningmean_class, runningmean_float);
    class_addmethod(runningmean_class, (t_method)runningmean_length, gensym("length"), A_FLOAT, 0);
    class_addmethod(runningmean_class, (t_method)runningmean_zero, gensym("clear"), 0);
}
/* end runningmean.c */

