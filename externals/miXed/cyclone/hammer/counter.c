/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is an entirely rewritten version of Joseph A. Sarlo's code.
   The most important changes are listed in "pd-lib-notes.txt" file.  */

/* Beware -- the max reference manual page for the counter object
   reflects mostly opcode max features.  Apparently counter works
   differently in cycling max (e.g. inlets 3 and 4).  But I am sick
   of checking -- I will not bother, until there is some feedback. */

#include "m_pd.h"
#include "common/loud.h"
#include "common/fitter.h"

#define COUNTER_UP      0
#define COUNTER_DOWN    1
#define COUNTER_UPDOWN  2
#define COUNTER_DEFMAX  0x7fffffff  /* CHECKED (man says otherwise) */

typedef struct _counter
{
    t_object   x_ob;
    int        x_count;
    int        x_maxcount;
    int        x_dir;
    int        x_inc;
    int        x_min;
    int        x_max;
    int        x_carrybang;
    int        x_minhitflag;
    int        x_maxhitflag;
    t_pd      *x_proxies[4];
    t_outlet  *x_out2;
    t_outlet  *x_out3;
    t_outlet  *x_out4;
} t_counter;

typedef struct _counter_proxy
{
    t_object    p_ob;
    t_counter  *p_master;
    void      (*p_bangmethod)(t_counter *x);
    void      (*p_floatmethod)(t_counter *x, t_float f);
} t_counter_proxy;

static t_class *counter_class;
static t_class *counter_proxy_class;

static void counter_up(t_counter *x)
{
    x->x_dir = COUNTER_UP;
    x->x_inc = 1;
}

static void counter_down(t_counter *x)
{
    /* CHECKED: no explicit minimum needed */
    x->x_dir = COUNTER_DOWN;
    x->x_inc = -1;
}

static void counter_updown(t_counter *x)
{
    /* CHECKED: neither explicit maximum, nor minimum needed */
    x->x_dir = COUNTER_UPDOWN;
    /* CHECKED: x->x_inc unchanged (continuation) */
}

static void counter_dir(t_counter *x, t_floatarg f)
{
    switch ((int)f)
    {
    case COUNTER_UP:
        counter_up(x);
	break;
    case COUNTER_DOWN:
        counter_down(x);
	break;
    case COUNTER_UPDOWN:
        counter_updown(x);
	break;
    default:
        counter_up(x);  /* CHECKED: invalid == default */
	/* CHECKED: no warning */
    }
}

static void counter_dobang(t_counter *x, int notjam)
{
    int offmin = 0, offmax = 0, onmin = 0, onmax = 0;
    /* CHECKED: carry-off is not sent if min >= max */
    /* LATER rethink (this is a hack) */
    if (x->x_min < x->x_max)
	offmin = x->x_minhitflag, offmax = x->x_maxhitflag;
    x->x_minhitflag = x->x_maxhitflag = 0;

    if (x->x_count < x->x_min)
    {
	if (x->x_inc == 1)
	{
	    /* min has changed, which should imply x->x_count == x->x_min */
	    loudbug_bug("counter_dobang (count < min)");
	}
	else if (x->x_dir == COUNTER_UPDOWN)
	{
	    x->x_inc = 1;
	    if ((x->x_count = x->x_min + 1) > x->x_max) x->x_count = x->x_min;
	}
	else if ((x->x_count = x->x_max) < x->x_min) x->x_count = x->x_min;
    }
    else if (x->x_count > x->x_max)
    {
	if (x->x_inc == -1)
	{
	    /* CHECKED: ignored */
	}
	else if (x->x_dir == COUNTER_UPDOWN)
	{
	    x->x_inc = -1;
	    if ((x->x_count = x->x_max - 1) < x->x_min) x->x_count = x->x_min;
	}
	else x->x_count = x->x_min;
    }

    if (x->x_count == x->x_min && x->x_inc == -1)
    {
	/* CHECKED: 'jam' inhibits middle outlets (unless carry-off)
	   carry-on is never sent if max < min, but sent if max == min */
	if (notjam
	    && x->x_min <= x->x_max)  /* LATER rethink (this is a hack) */
	    onmin = 1;
    }
    else if (x->x_count == x->x_max && x->x_inc == 1)
    {
	/* CHECKED: this counter is never reset (and goes up to INT_MAX)
	   -- neither after dir change, nor after max change */
	x->x_maxcount++;  /* CHECKED: 'jam' does the increment */
	outlet_float(x->x_out4, x->x_maxcount);
	/* CHECKED: 'jam' inhibits middle outlets (unless carry-off)
	   carry-on is never sent if max < min, but sent if max == min */
	if (notjam
	    && x->x_min <= x->x_max)  /* LATER rethink (this is a hack) */
	    onmax = 1;
    }

    /* CHECKED: outlets deliver in right-to-left order */
    if (onmax)
    {
	if (x->x_carrybang) outlet_bang(x->x_out3);
	else
	{
	    outlet_float(x->x_out3, 1);
	    x->x_maxhitflag = 1;
	}
    }
    else if (offmax) outlet_float(x->x_out3, 0);
    else if (onmin)
    {
	if (x->x_carrybang) outlet_bang(x->x_out2);
	else
	{
	    outlet_float(x->x_out2, 1);
	    x->x_minhitflag = 1;
	}
    }
    else if (offmin) outlet_float(x->x_out2, 0);

    outlet_float(((t_object *)x)->ob_outlet, x->x_count);
}

static void counter_bang(t_counter *x)
{
    x->x_count += x->x_inc;
    counter_dobang(x, 1);
}

static void counter_float(t_counter *x, t_float dummy)
{
    counter_bang(x);
}

/* CHECKED: out-of-range values are ignored */
/* CHECKED: 'down, set 3, up, bang' gives 5 */
static void counter_set(t_counter *x, t_floatarg f)
{
    int i = (int)f;
    if (i >= x->x_min && i <= x->x_max)
	x->x_count = i - x->x_inc;
}

/* CHECKED: out-of-range values are ignored */
static void counter_jam(t_counter *x, t_floatarg f)
{
    int i = (int)f;
    if (i >= x->x_min && i <= x->x_max)
    {
	x->x_count = i;
	counter_dobang(x, 0);
    }
}

/* CHECKED: sends max carry on/off in any mode  */
static void counter_inc(t_counter *x)
{
    int tempdir = x->x_dir;
    int tempinc = x->x_inc;
    counter_up(x);
    counter_bang(x);
    x->x_dir = tempdir;
    x->x_inc = tempinc;
}

/* CHECKED: sends min carry on/off in any mode */
static void counter_dec(t_counter *x)
{
    int tempdir = x->x_dir;
    int tempinc = x->x_inc;
    counter_down(x);
    counter_bang(x);
    x->x_dir = tempdir;
    x->x_inc = tempinc;
}

/* CHECKED: min can be set over max */
static void counter_min(t_counter *x, t_floatarg f)
{
    /* CHECKED: min change always sets count to min and bangs */
    /* do not use counter_jam() here -- avoid range checking */
    x->x_count = x->x_min = (int)f;
    counter_dobang(x, 0);
}

/* CHECKED: max can be set below min */
static void counter_max(t_counter *x, t_floatarg f)
{
    x->x_max = (int)f;
}

static void counter_carrybang(t_counter *x)
{
    x->x_carrybang = 1;
}

static void counter_carryint(t_counter *x)
{
    x->x_carrybang = 0;
}

/* CHECKED: up/down switch */
static void counter_bang1(t_counter *x)
{
    if (x->x_dir == COUNTER_UP)
        counter_down(x);
    else if (x->x_dir == COUNTER_DOWN)
        counter_up(x);
    else
	x->x_inc = -x->x_inc;  /* CHECKED */
}

/* CHECKED */
static void counter_bang2(t_counter *x)
{
    counter_set(x, x->x_min);
}

/* CHECKED: out-of-range values are accepted (LATER rethink) */
/* CHECKED: no resetting of min, nor of max (contrary to the man) */
/* CHECKED: 'down, float2 3, up, bang' gives 3 (LATER rethink) */
static void counter_float2(t_counter *x, t_floatarg f)
{
    counter_set(x, f);  /* FIXME */
}

/* CHECKED */
static void counter_bang3(t_counter *x)
{
    counter_jam(x, x->x_min);
}

/* CHECKED: out-of-range values are accepted (LATER rethink) */
/* CHECKED: no resetting of min, nor of max (contrary to the man) */
static void counter_float3(t_counter *x, t_floatarg f)
{
    counter_jam(x, f);  /* FIXME */
}

/* CHECKED */
static void counter_bang4(t_counter *x)
{
    counter_set(x, x->x_max);
}

static void counter_proxy_bang(t_counter_proxy *x)
{
    x->p_bangmethod(x->p_master);
}

static void counter_proxy_float(t_counter_proxy *x, t_float f)
{
    x->p_floatmethod(x->p_master, f);
}

static void counter_free(t_counter *x)
{
    int i;
    for (i = 0; i < 4; i++)
	if (x->x_proxies[i]) pd_free(x->x_proxies[i]);
}

static void *counter_new(t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    t_counter *x = (t_counter *)pd_new(counter_class);
    t_counter_proxy **pp = (t_counter_proxy **)x->x_proxies;
    int i1 = (int)f1;
    int i2 = (int)f2;
    int i3 = (int)f3;
    int i;
    static int warned = 0;
    if (fittermax_get() && !warned)
    {
	post("warning: counter is not fully compatible,\
 please report differences");
	warned = 1;
    }
    x->x_dir = COUNTER_UP;
    x->x_inc = 1;  /* previous value required by counter_dir() */
    x->x_min = 0;
    x->x_max = COUNTER_DEFMAX;
    if (i3) x->x_dir = i1, x->x_min = i2, x->x_max = i3;
    else if (i2) x->x_min = i1, x->x_max = i2;
    else if (i1) x->x_max = i1;
    x->x_carrybang = 0;  /* CHECKED */
    x->x_minhitflag = x->x_maxhitflag = 0;
    x->x_maxcount = 0;
    counter_dir(x, x->x_dir);
    /* CHECKED: [counter 1 <min> <max>] starts from <max> */
    x->x_count = (x->x_dir == COUNTER_DOWN ? x->x_max : x->x_min);
    for (i = 0; i < 4; i++)
    {
	x->x_proxies[i] = pd_new(counter_proxy_class);
	((t_counter_proxy *)x->x_proxies[i])->p_master = x;
	inlet_new((t_object *)x, x->x_proxies[i], 0, 0);
    }
    (*pp)->p_bangmethod = counter_bang1;
    (*pp++)->p_floatmethod = counter_dir;  /* CHECKED: same as dir */
    (*pp)->p_bangmethod = counter_bang2;
    (*pp++)->p_floatmethod = counter_float2;
    (*pp)->p_bangmethod = counter_bang3;
    (*pp++)->p_floatmethod = counter_float3;
    (*pp)->p_bangmethod = counter_bang4;
    (*pp++)->p_floatmethod = counter_max;  /* CHECKED: same as max */
    outlet_new((t_object *)x, &s_float);
    x->x_out2 = outlet_new((t_object *)x, &s_anything);  /* float/bang */
    x->x_out3 = outlet_new((t_object *)x, &s_anything);  /* float/bang */
    x->x_out4 = outlet_new((t_object *)x, &s_float);
    return (x);
}

void counter_setup(void)
{
    counter_class = class_new(gensym("counter"),
			      (t_newmethod)counter_new,
			      (t_method)counter_free,
			      sizeof(t_counter), 0,
			      A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(counter_class, counter_bang);
    class_addfloat(counter_class, counter_float);
    class_addmethod(counter_class, (t_method)counter_bang,
		    gensym("next"), 0);
    class_addmethod(counter_class, (t_method)counter_set,
		    gensym("set"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_set,
		    gensym("goto"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_jam,
		    gensym("jam"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_up,
		    gensym("up"), 0);
    class_addmethod(counter_class, (t_method)counter_down,
		    gensym("down"), 0);
    class_addmethod(counter_class, (t_method)counter_updown,
		    gensym("updown"), 0);
    class_addmethod(counter_class, (t_method)counter_inc,
		    gensym("inc"), 0);
    class_addmethod(counter_class, (t_method)counter_dec,
		    gensym("dec"), 0);
    class_addmethod(counter_class, (t_method)counter_min,
		    gensym("min"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_max,
		    gensym("max"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_carrybang,
		    gensym("carrybang"), 0);
    class_addmethod(counter_class, (t_method)counter_carryint,
		    gensym("carryint"), 0);
    counter_proxy_class = class_new(gensym("_counter_proxy"), 0, 0,
				    sizeof(t_counter_proxy),
				    CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(counter_proxy_class, counter_proxy_bang);
    class_addfloat(counter_proxy_class, counter_proxy_float);
    fitter_setup(counter_class, 0);
}
