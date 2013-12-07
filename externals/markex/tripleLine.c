/*
 * Copyright (c) 1997-1999 Mark Danks.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt" in this distribution.
 */

#include "m_pd.h"

/* -------------------------- tripleLine ------------------------------ */

/* instance structure */

static t_class *tripleLine_class;

typedef struct _tripleLine
{
	t_object    x_obj;			/* obligatory object header */
	t_clock		*clock;
	float		grain;
	float		destTime;
	float		curTime;
	float		curVal[3];
	float		destVal[3];
	float		stepVal[3];
	t_outlet    *t_out1;	    /* the outlet */
} t_tripleLine;

static void tripleTick(t_tripleLine *x)
{
	t_atom argv[3];
	int i;
	x->curTime += x->grain;
	if (x->curTime >= x->destTime)
	{
		for (i = 0; i < 3; i++)
		{
			x->curVal[i] = x->destVal[i];
			SETFLOAT((&argv[i]), x->curVal[i]);
		}
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			x->curVal[i] += x->stepVal[i];
			SETFLOAT((&argv[i]), x->curVal[i]);
		}
	   	clock_delay(x->clock, x->grain);
	}

	outlet_list(x->t_out1, &s_list, 3, argv);
}

static void setLineParameters(t_tripleLine *x, t_symbol *s, int argc, t_atom *argv)
{
    float numCounts;
	int i;
    t_atom newargv[3];
	float time = x->destTime;
	
	clock_unset(x->clock);
	
	if (argc == 4) time = atom_getfloat(&argv[3]);
	else if (argc != 3)
	{
		error("tripleLine: requires 3 or 4 args");
		return;
	}

	x->destTime = time;
	numCounts = time / x->grain;
	if (x->destTime <= 0.)
	{
		x->curTime = 0;
		for (i = 0; i < 3; i++)
		{
			x->curVal[i] = x->destVal[i] = atom_getfloat(&argv[i]);
			SETFLOAT((&newargv[i]), x->curVal[i]);
		}
		outlet_list(x->t_out1, &s_list, 3, newargv);
		return;
	}
	else
	{
		for (i = 0; i < 3; i++)
		{
			x->destVal[i] = atom_getfloat(&argv[i]);
			x->stepVal[i] = (x->destVal[i] - x->curVal[i]) / numCounts;
			SETFLOAT((&newargv[i]), x->curVal[i]);
		}
	}
	x->curTime = 0;

   	clock_delay(x->clock, x->grain);
	outlet_list(x->t_out1, &s_list, 3, newargv);
}

static void tripleLine_free(t_tripleLine *x)
{
    clock_free(x->clock);
}

static void *tripleLine_new(t_floatarg time, t_floatarg grain) /* init vals in struc */
{
	int i;
    t_tripleLine *x = (t_tripleLine *)pd_new(tripleLine_class);
	x->curTime = 0;
	x->destTime = time;
    x->clock = clock_new(x, (t_method)tripleTick);
	for (i = 0; i < 3; i++)
	{
		x->curVal[i] = x->destVal[i] = x->stepVal[i] = 0.;
	}
	if (grain <= 0) grain = 50;
	x->grain = (float)grain;

    x->t_out1 = outlet_new(&x->x_obj, 0);
    return(x);
}

void tripleLine_setup(void)
{
    tripleLine_class = class_new(gensym("tripleLine"), (t_newmethod)tripleLine_new,
			(t_method)tripleLine_free, sizeof(t_tripleLine), 0, A_DEFFLOAT, A_DEFFLOAT, 0);

    class_addmethod(tripleLine_class, (t_method)setLineParameters,
    	    &s_list, A_GIMME, A_NULL); 

#if PD_MINOR_VERSION < 37 
	class_sethelpsymbol(tripleLine_class, gensym("tripleLine-help.pd"));
#endif
}


