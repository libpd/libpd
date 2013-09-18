/* --------------------- threshold ----------------------------- */

#include "defines.h"

static t_class *threshold_class;

typedef struct _threshold
{
    t_object x_obj;
    t_outlet *x_outlet1;    	/* bang out for high thresh */
    t_outlet *x_outlet2;    	/* bang out for low thresh */
    t_clock *x_clock;	    	/* wakeup for message output */
    int x_state;    			/* 1 = high, 0 = low */
    t_float x_hithresh;	    	/* value of high threshold */
    t_float x_lothresh;	    	/* value of low threshold */
    t_float x_deadwait;	    	/* msec remaining in dead period */
    t_float x_hideadtime;	    	/* hi dead */
    t_float x_lodeadtime;	    	/* lo dead */
} t_threshold;

static void threshold_tick(t_threshold *x);
static void threshold_set(t_threshold *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime);

static t_threshold *threshold_new(t_floatarg hithresh,
    t_floatarg hideadtime, t_floatarg lothresh, t_floatarg lodeadtime)
{
    t_threshold *x = (t_threshold *)
    	pd_new(threshold_class);
    x->x_state = 0;		/* low state */
    x->x_deadwait = 0;		/* no dead time */
    x->x_clock = clock_new(x, (t_method)threshold_tick);
    x->x_outlet1 = outlet_new(&x->x_obj, gensym("bang"));
    x->x_outlet2 = outlet_new(&x->x_obj, gensym("bang"));
    threshold_set(x, hithresh, hideadtime, lothresh, lodeadtime);
    return (x);
}

    /* "set" message to specify thresholds and dead times */
static void threshold_set(t_threshold *x,
    t_floatarg hithresh, t_floatarg hideadtime,
    t_floatarg lothresh, t_floatarg lodeadtime)
{
    if (lothresh > hithresh)
    	lothresh = hithresh;
    x->x_hithresh = hithresh;
    x->x_hideadtime = hideadtime;
    x->x_lothresh = lothresh;
    x->x_lodeadtime = lodeadtime;
}

    /* number in inlet sets state -- note incompatible with JMAX which used
    "int" message for this, impossible here because of auto signal conversion */
static void threshold_ft1(t_threshold *x, t_floatarg f)
{
    x->x_state = (f != 0);
    x->x_deadwait = 0;
}

static void threshold_tick(t_threshold *x)	
{
    if (x->x_state)
    	outlet_bang(x->x_outlet1);
    else outlet_bang(x->x_outlet2);
}

static void threshold_perform(t_threshold *x, t_float in)
{
    if (x->x_deadwait > 0)
    	x->x_deadwait -= 1;
    else if (x->x_state)
    {
    	    /* we're high; look for low sample */
	    if (in < x->x_lothresh)
	    {
		clock_delay(x->x_clock, 0L);
		x->x_state = 0;
		x->x_deadwait = x->x_lodeadtime;
	    }
    }
    else
    {
    	    /* we're low; look for high sample */
	    if (in >= x->x_hithresh)
	    {
		clock_delay(x->x_clock, 0L);
		x->x_state = 1;
		x->x_deadwait = x->x_hideadtime;
	    }
    }
}


static void threshold_ff(t_threshold *x)
{
    clock_free(x->x_clock);
}

void threshold_setup( void)
{
    threshold_class = class_new(gensym("threshold"),
    	(t_newmethod)threshold_new, (t_method)threshold_ff,
	sizeof(t_threshold), 0,
	    A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addfloat(threshold_class, (t_method)threshold_perform);   
	class_addmethod(threshold_class, (t_method)threshold_set,
    	gensym("set"), A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
    class_addmethod(threshold_class, (t_method)threshold_ft1,
    	gensym("ft1"), A_FLOAT, 0);
}
