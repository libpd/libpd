/* Copyright (c) 2002-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* This is a modified version of Joseph A. Sarlo's code.
   The most important changes are listed in "pd-lib-notes.txt" file.  */

#include "m_pd.h"

typedef struct _Bucket
{
    t_object    x_ob;
    int         x_numbucks;
    t_float    *x_bucks;   /* CHECKED: no limit */
    t_outlet  **x_outs;
    short int   x_frozen;  /* 0 for thawed, 1 for frozen */
    short int   x_dir;     /* 0 for L2R, 1 for R2L */
} t_Bucket;

static t_class *Bucket_class;

static void Bucket_bang(t_Bucket *x)
{
    int i = x->x_numbucks;
    /* CHECKED: outlets output in right-to-left order */
    while (i--) outlet_float(x->x_outs[i], x->x_bucks[i]);
}

static void Bucket_float(t_Bucket *x, t_float val)
{
    int i;

    if (!x->x_frozen)
	Bucket_bang(x);
    if (!x->x_dir)
    {
	for (i = x->x_numbucks - 1; i > 0; i--)
	    x->x_bucks[i] = x->x_bucks[i - 1];
	x->x_bucks[0] = val;
    }
    else
    {
	for (i = 0; i < x->x_numbucks - 1; i++)
	    x->x_bucks[i] = x->x_bucks[i + 1];
	x->x_bucks[x->x_numbucks - 1] = val;
    }
}

static void Bucket_freeze(t_Bucket *x)
{
    x->x_frozen = 1;
}

static void Bucket_thaw(t_Bucket *x)
{
    x->x_frozen = 0;
}

static void Bucket_roll(t_Bucket *x)
{
    if (x->x_dir)
	Bucket_float(x, x->x_bucks[0]);
    else
	Bucket_float(x, x->x_bucks[x->x_numbucks - 1]);
}

static void Bucket_rtol(t_Bucket *x)
{
    x->x_dir = 1;
}

static void Bucket_ltor(t_Bucket *x)
{
    x->x_dir = 0;
}

static void Bucket_set(t_Bucket *x, t_floatarg f)
{
    int i = x->x_numbucks;
    while (i--) x->x_bucks[i] = f;
    if (!x->x_frozen)  /* CHECKED */
	Bucket_bang(x);
}

static void Bucket_free(t_Bucket *x)
{
    if (x->x_bucks)
	freebytes(x->x_bucks, x->x_numbucks * sizeof(*x->x_bucks));
    if (x->x_outs)
	freebytes(x->x_outs, x->x_numbucks * sizeof(*x->x_outs));
}

static void *Bucket_new(t_floatarg val)
{
    t_Bucket *x;
    int i, nbucks = (int)val;
    t_float *bucks;
    t_outlet **outs;
    if (nbucks < 1)
	nbucks = 1;
    if (!(bucks = (t_float *)getbytes(nbucks * sizeof(*bucks))))
	return (0);
    if (!(outs = (t_outlet **)getbytes(nbucks * sizeof(*outs))))
    {
	freebytes(bucks, nbucks * sizeof(*bucks));
	return (0);
    }
    x = (t_Bucket *)pd_new(Bucket_class);
    x->x_numbucks = nbucks;
    x->x_bucks = bucks;
    x->x_outs = outs;
    x->x_frozen = 0;
    x->x_dir = 0;
    while (nbucks--) *outs++ = outlet_new((t_object *)x, &s_float);
    return (x);
}

void Bucket_setup(void)
{
    Bucket_class = class_new(gensym("Bucket"),
			     (t_newmethod)Bucket_new,
			     (t_method)Bucket_free,
			     sizeof(t_Bucket), 0, A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Bucket_new, gensym("bucket"), A_DEFFLOAT, 0);
    class_addcreator((t_newmethod)Bucket_new, gensym("cyclone/bucket"), A_DEFFLOAT, 0);
    class_addbang(Bucket_class, Bucket_bang);
    class_addfloat(Bucket_class, Bucket_float);
    class_addmethod(Bucket_class, (t_method)Bucket_freeze, gensym("freeze"), 0);
    class_addmethod(Bucket_class, (t_method)Bucket_thaw, gensym("thaw"), 0);
    class_addmethod(Bucket_class, (t_method)Bucket_ltor, gensym("L2R"), 0);
    class_addmethod(Bucket_class, (t_method)Bucket_rtol, gensym("R2L"), 0);
    /* CHECKED (refman error) roll has no argument */
    class_addmethod(Bucket_class, (t_method)Bucket_roll, gensym("roll"), 0);
    /* 3.5 additions */
    class_addmethod(Bucket_class, (t_method)Bucket_set,
		    gensym("set"), A_FLOAT, 0);
    class_addmethod(Bucket_class, (t_method)Bucket_ltor, gensym("l2r"), 0);
    class_addmethod(Bucket_class, (t_method)Bucket_rtol, gensym("r2l"), 0);
}

void bucket_setup(void)
{
    Bucket_setup();
}
