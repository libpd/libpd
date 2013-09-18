/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- alternate ------------------------------ */

/* instance structure */
static t_class *average_class;

#define MAX_NUMBERS 100

typedef struct _average
{
	t_object    x_obj;	        /* obligatory object header */
	int	        a_total;	    /* number of numbers to average */
	int	        a_whichNum;	    /* which number are pointing at */
	float	    a_numbers[MAX_NUMBERS]; /* numbers to average */
	t_outlet    *t_out1;	    /* the outlet */
} t_average;

void average_bang(t_average *x)
{
    float average = 0.0f;
    int n;
    
    for (n = 0; n < x->a_total; n++) average = average + x->a_numbers[n];
    average = average / (float)x->a_total;
     
    outlet_float(x->t_out1, average);
}

void average_float(t_average *x, t_floatarg n)
{
    if (x->a_whichNum >= x->a_total) x->a_whichNum = 0;
    x->a_numbers[x->a_whichNum] = n;
    x->a_whichNum++;
    average_bang(x);
}

void average_total(t_average *x, t_float f)
{
    if (f) x->a_total = (int)f;
    else x->a_total = 10;
    if (x->a_total > MAX_NUMBERS) {
        logpost(x, 2, "[average]: argument set max numbers greater than %i, setting to %i",
                MAX_NUMBERS, MAX_NUMBERS);
        x->a_total = MAX_NUMBERS;
    }
}

void average_reset(t_average *x, t_floatarg newVal)
{
    int n;  
    for (n=0; n < MAX_NUMBERS; n ++) x->a_numbers[n] = newVal;
}

void average_clear(t_average *x)
{
    int n;    
    for ( n = 0; n < MAX_NUMBERS; n ++) x->a_numbers[n] = 0.0f;
}

void *average_new(t_floatarg f) /* init vals in struc */
{
    t_average *x = (t_average *)pd_new(average_class);
    x->t_out1 = outlet_new(&x->x_obj, 0);
    inlet_new(&x->x_obj, &x->x_obj.ob_pd, &s_float, gensym("fl1"));
    average_clear(x);
    average_total(x, f);
    x->a_whichNum = 0;
    return (x);
}

void average_setup(void)
{
    average_class = class_new(gensym("average"), (t_newmethod)average_new, 0,
    	    	    	sizeof(t_average), 0, A_DEFFLOAT, 0);
    class_addbang(average_class, (t_method)average_bang);
    class_addfloat(average_class, (t_method)average_float);
    class_addmethod(average_class, (t_method)average_total, gensym("fl1"), A_FLOAT, 0);
    class_addmethod(average_class, (t_method)average_clear, gensym("clear"), A_NULL);
    class_addmethod(average_class, (t_method)average_reset, gensym("reset"), A_FLOAT,  0);

	#if PD_MINOR_VERSION < 37 

#endif
}

