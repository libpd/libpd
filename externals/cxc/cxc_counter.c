/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "GEM.LICENSE.TERMS" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- counter ------------------------------ */

/* instance structure */
static t_class *counter_class;

typedef struct _counter
{
	t_object    x_obj;	    /* obligatory object header */
	int	        c_current;	/* current number */
	int	        c_high;	    /* highest number */
	int	        c_low;	    /* lowest number */
	int	        c_updown;	/* 0 = going up, 1 = going down */
	int	        c_dir;	    /* counter dir. 1=up, 2=down, 3=up/down */
	t_outlet    *t_out1;	/* the outlet */
	t_outlet    *t_out2;	/* the outlet */
} t_counter;

void counter_bang(t_counter *x)
{
	int sendBang = 0;
    switch(x->c_dir)
    {
		// count up
	    case 1:
            x->c_current++;
	        if (x->c_current > x->c_high)
				x->c_current = x->c_low;
	        else if (x->c_current < x->c_low)
				x->c_current = x->c_low;
			else if (x->c_current == x->c_high)
				sendBang = 1;
	        break;
	    // count down
		case 2:
	        x->c_current--;
	        if (x->c_current < x->c_low)
				x->c_current = x->c_high;
	        else if (x->c_current > x->c_high)
				x->c_current = x->c_high;
			else if (x->c_current == x->c_low)
				sendBang = 1;
	        break;
	    // count up and down
		case 3:
	        // going up
			if (x->c_updown == 0)
	        {
                x->c_current++;
		        if (x->c_current > x->c_high)
		        {
		            x->c_current = x->c_high - 1;
		            x->c_updown = 1;
		        }
		        else if (x->c_current < x->c_low)
					x->c_current = x->c_low;
				else if (x->c_current == x->c_high)
					sendBang = 1;
	        }
	        // going down
			else if (x->c_updown == 1)
	        {
                x->c_current--;
		        if (x->c_current < x->c_low)
		        {
		            x->c_current = x->c_low + 1;
		            x->c_updown = 0;
		        }
		        else if (x->c_current > x->c_high)
					x->c_current = x->c_high;
				else if (x->c_current == x->c_low)
					sendBang = 1;
	        }
	        else
	        {
		        error("up/down wrong");
		        return;
	        }
	        break;
	    default:
	        error("dir wrong");
	        return;
    }
    outlet_float(x->t_out1, (float)x->c_current);
	if (sendBang)
		outlet_bang(x->t_out2);
}

void counter_dir(t_counter *x, t_floatarg n)
{
    if (n == 1 || n == 2 || n == 3) x->c_dir = (int)n;
    else error("bad dir");
}

void counter_high(t_counter *x, t_floatarg n)
{
    x->c_high = (int)n;
}

void counter_low(t_counter *x, t_floatarg n)
{
    x->c_low = (int)n;
}

void counter_reset(t_counter *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc)
    {
	    switch(x->c_dir)
	    {
	        case 1:
		        x->c_current = x->c_low;
		        break;
	        case 2:
		        x->c_current = x->c_high;
		        break;
	        case 3:
		        if (x->c_updown == 0) x->c_current = x->c_low;
		        else if (x->c_updown == 1) x->c_current = x->c_high;
		        break;
	        default:
		        return;
	    }
    }
    else
    {
	    switch(argv[0].a_type)
	    {
	        case A_FLOAT :
		        x->c_current = (int)argv[0].a_w.w_float;
		        break;
	        default :
		        error ("counter: reset not float");
		        break;
	    }
    }
    outlet_float(x->t_out1, (float)x->c_current);
}

void counter_clear(t_counter *x, t_symbol *s, int argc, t_atom *argv)
{
    if (!argc)
    {
	    switch(x->c_dir)
	    {
	        case 1:
		        x->c_current = x->c_low - 1;
		        break;
	        case 2:
		        x->c_current = x->c_high + 1;
		        break;
	        case 3:
		        if (x->c_updown == 0) x->c_current = x->c_low - 1;
		        else if (x->c_updown == 1) x->c_current = x->c_high + 1;
		        break;
	        default:
		        break;
	    }
    }
    else
    {
	    switch(argv[0].a_type)
	    {
	        case A_FLOAT :
		        x->c_current = (int)argv[0].a_w.w_float - 1;
		        break;
	        default :
		        error ("counter: reset not float");
		        break;
	    }
    }
}

void *counter_new(t_floatarg f, t_floatarg g, t_floatarg h) /* init vals in struc */
{
    t_counter *x = (t_counter *)pd_new(counter_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    x->t_out2 = outlet_new(&x->x_obj, 0);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl2"));
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl3"));
    x->c_current = 0;
    x->c_updown = 0;
    if (h)
    {
	    counter_low(x, f);
	    counter_high(x, g);
	    counter_dir(x, h);
    }
    else if (g)
    {
	    x->c_dir = 1;
	    counter_low(x, f);
	    counter_high(x, g);
    }
    else if (f)
    {
	    x->c_dir = x->c_low = 1;
	    counter_high(x, f);
    }
    else
    {
	    x->c_dir = x->c_low = 1;
	    x->c_high = 10;
    }
    return (x);
}

void cxc_counter_setup(void)
{
    counter_class = class_new(gensym("cxc_counter"), (t_newmethod)counter_new, 0,
    	    sizeof(t_counter), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addbang(counter_class, (t_method)counter_bang);
    class_addmethod(counter_class, (t_method)counter_dir, gensym("fl1"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_low, gensym("fl2"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_high, gensym("fl3"), A_FLOAT, 0);
    class_addmethod(counter_class, (t_method)counter_reset, gensym("reset"), A_GIMME, 0);
    class_addmethod(counter_class, (t_method)counter_clear, gensym("clear"), A_GIMME, 0);
}
