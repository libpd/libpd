/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include "m_pd.h"
#include "common/loud.h"
#include "common/fitter.h"
#include "unstable/fragile.h"
#include "sickle/sic.h"

#ifdef KRZYSZCZ
//#define MATRIX_DEBUG
#endif

#define MATRIX_DEFGAIN  0.  /* CHECKED */
#define MATRIX_DEFRAMP  0.  /* CHECKED */

#define MATRIX_GAINEPSILON  1e-20f
#define MATRIX_MINRAMP      .001  /* LATER rethink */

typedef struct _matrix
{
    t_sic      x_sic;
    int        x_ninlets;
    int        x_noutlets;
    int        x_nblock;
    int        x_maxblock;
    t_float  **x_ivecs;
    t_float  **x_ovecs;
    t_float  **x_osums;
    int        x_ncells;
    int       *x_cells;
    t_outlet  *x_dumpout;
    /* The following fields are specific to nonbinary mode, i.e. we keep them
       unallocated in binary mode.  This is CHECKED to be incompatible:  c74
       always accepts (and reports) gains and ramps, although they are actually
       meaningless in binary mode, and switching modes is not supported. */
    float      x_defgain;
    float     *x_gains;  /* target gains */
    float      x_deframp;
    float     *x_ramps;
    float      x_ksr;
    float     *x_coefs;  /* current coefs */
    float     *x_incrs;
    float     *x_bigincrs;
    int       *x_remains;
} t_matrix;

typedef void (*t_matrix_cellfn)(t_matrix *x, int indx, int ondx,
				int onoff, float gain);

static t_class *matrix_class;
static t_symbol *matrixps_matrixtilde;

/* called only in nonbinary mode;  LATER deal with changing nblock/ksr */
static void matrix_retarget(t_matrix *x, int cellndx)
{
    float target = (x->x_cells[cellndx] ? x->x_gains[cellndx] : 0.);
    if (x->x_ramps[cellndx] < MATRIX_MINRAMP)
    {
	x->x_coefs[cellndx] = target;
	x->x_remains[cellndx] = 0;
    }
    else
    {
    	x->x_remains[cellndx] =
	    x->x_ramps[cellndx] * x->x_ksr + 0.5;  /* LATER rethink */
    	x->x_incrs[cellndx] =
	    (target - x->x_coefs[cellndx]) / (float)x->x_remains[cellndx];
	x->x_bigincrs[cellndx] = x->x_nblock * x->x_incrs[cellndx];
    }
}

static void matrix_float(t_matrix *x, t_float f)
{
    loud_nomethod((t_pd *)x, &s_float);  /* CHECKED */
}

static void matrix_list(t_matrix *x, t_symbol *s, int ac, t_atom *av)
{
    int indx, ondx, cellndx, onoff;
    float gain;
    if (ac < 3)
	return;  /* CHECKED list silently ignored if ac < 3 */
    /* CHECKED floats silently clipped, symbols converted to 0 */
    indx = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
    if (indx < 0 || indx >= x->x_ninlets)
    {  /* CHECKED */
	loud_error((t_pd *)x, "invalid inlet number %d", indx);
	return;
    }
    ac--; av++;
    /* CHECKED floats silently clipped, symbols converted to 0 */
    ondx = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
    if (ondx < 0 || ondx >= x->x_noutlets)
    {  /* CHECKED */
	loud_error((t_pd *)x, "invalid outlet number %d", ondx);
	return;
    }
    cellndx = indx * x->x_noutlets + ondx;
    ac--; av++;
    /* CHECKED negative gain used in nonbinary mode, accepted as 1 in binary */
    gain = (av->a_type == A_FLOAT ? av->a_w.w_float : 0.);
    onoff = (gain < -MATRIX_GAINEPSILON || gain > MATRIX_GAINEPSILON);
    x->x_cells[cellndx] = onoff;
    if (x->x_gains)
    {
	if (onoff)  /* CHECKME */
	    x->x_gains[cellndx] = gain;
	ac--; av++;
	if (ac)
	{
	    float ramp = (av->a_type == A_FLOAT ? av->a_w.w_float : 0.);
	    x->x_ramps[cellndx] = (ramp < MATRIX_MINRAMP ? 0. : ramp);
	}
	matrix_retarget(x, cellndx);
    }
}

static void matrix_clear(t_matrix *x)
{
    int i;
    for (i = 0; i < x->x_ncells; i++)
	x->x_cells[i] = 0;
}

static void matrix_set(t_matrix *x, t_floatarg f1, t_floatarg f2)
{
    int i, onoff;
    float gain = f1;
    static int warned = 0;
    if (fittermax_get() && !warned)
    {
	fittermax_warning(*(t_pd *)x, "'set' not supported in Max");
	warned = 1;
    }
    onoff = (gain < -MATRIX_GAINEPSILON || gain > MATRIX_GAINEPSILON);
    for (i = 0; i < x->x_ncells; i++)
	x->x_cells[i] = onoff;
    if (x->x_gains)
    {
	float ramp = (f2 < MATRIX_MINRAMP ? 0. : f2);
	for (i = 0; i < x->x_ncells; i++)
	{
	    if (onoff)  /* LATER rethink */
		x->x_gains[i] = gain;
	    x->x_ramps[i] = ramp;
	    matrix_retarget(x, i);
	}
    }
}

/* CHECKED c74's refman and help patch are wrong about int pairs --
   the actual syntax is "[dis]connect indx ondx1 [ondx2 [ondx3..." */
static void matrix_connect(t_matrix *x, t_symbol *s, int ac, t_atom *av)
{
    int onoff = (s == gensym("connect")), indx, celloffset;
    if (ac < 2)
	return;  /* CHECKED */
    /* CHECKED floats silently clipped, symbols converted to 0 */
    indx = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
    if (indx < 0 || indx >= x->x_ninlets)
    {  /* CHECKED */
	loud_error((t_pd *)x, "invalid inlet number %d", indx);
	return;
    }
    celloffset = indx * x->x_noutlets;
    ac--; av++;
    while (ac)
    {
	/* CHECKED floats silently clipped, symbols converted to 0 */
	int cellndx, ondx = (av->a_type == A_FLOAT ? (int)av->a_w.w_float : 0);
	if (ondx < 0 || ondx >= x->x_noutlets)
	{  /* CHECKED */
	    loud_error((t_pd *)x, "invalid outlet number %d", ondx);
	    return;
	}
	cellndx = celloffset + ondx;
	x->x_cells[cellndx] = onoff;
	if (x->x_gains)
	    matrix_retarget(x, cellndx);
	ac--; av++;
    }
}

/* CHECKED active ramps are not retargeted */
static void matrix_ramp(t_matrix *x, t_floatarg f)
{
    if (x->x_ramps)
    {
	int i;
	x->x_deframp = (f < MATRIX_MINRAMP ? 0. : f);
	/* CHECKED cell-specific ramps are lost */
	for (i = 0; i < x->x_ncells; i++)
	    x->x_ramps[i] = x->x_deframp;
    }
}

static t_int *matrix01_perform(t_int *w)
{
    t_matrix *x = (t_matrix *)(w[1]);
    int nblock = (int)(w[2]);
    t_float **ivecs = x->x_ivecs;
    t_float **ovecs = x->x_ovecs;
    t_float **osums = x->x_osums;
    int *cellp = x->x_cells;
    int indx = x->x_ninlets;
    while (indx--)
    {
	t_float *ivec = *ivecs++;
	t_float **ovecp = osums;
	int ondx = x->x_noutlets;
	while (ondx--)
	{
	    if (*cellp++)
	    {
		t_float *in = ivec;
		t_float *out = *ovecp;
		int sndx = nblock;
		while (sndx--)
		    *out++ += *in++;
	    }
	    ovecp++;
	}
    }
    osums = x->x_osums;
    indx = x->x_noutlets;
    while (indx--)
    {
	t_float *in = *osums++;
	t_float *out = *ovecs++;
	int sndx = nblock;
	while (sndx--)
	{
	    *out++ = *in;
	    *in++ = 0.;
	}
    }
    return (w + 3);
}

static t_int *matrixnb_perform(t_int *w)
{
    t_matrix *x = (t_matrix *)(w[1]);
    int nblock = (int)(w[2]);
    t_float **ivecs = x->x_ivecs;
    t_float **ovecs = x->x_ovecs;
    t_float **osums = x->x_osums;
    int *cellp = x->x_cells;
    float *gainp = x->x_gains;
    float *coefp = x->x_coefs;
    float *incrp = x->x_incrs;
    float *bigincrp = x->x_bigincrs;
    int *nleftp = x->x_remains;
    int indx = x->x_ninlets;
    while (indx--)
    {
	t_float *ivec = *ivecs++;
	t_float **ovecp = osums;
	int ondx = x->x_noutlets;
	while (ondx--)
	{
	    t_float *in = ivec;
	    t_float *out = *ovecp;
	    float nleft = *nleftp;
	    int sndx = nblock;
	    if (nleft >= nblock)
	    {
		float coef = *coefp;
		float incr = *incrp;
		if ((*nleftp -= nblock) == 0)
		    *coefp = (*cellp ? *gainp : 0.);
		else
		    *coefp += *bigincrp;
		while (sndx--)
		    *out++ += *in++ * coef, coef += incr;
	    }
	    else if (nleft > 0)
	    {
		float coef = *coefp;
		float incr = *incrp;
		sndx -= nleft;
		do
		    *out++ += *in++ * coef, coef += incr;
		while (--nleft);
		if (*cellp)
		{
		    coef = *coefp = *gainp;
		    while (sndx--)
			*out++ += *in++ * coef;
		}
		else *coefp = 0.;
		*nleftp = 0;
	    }
	    else if (*cellp)
	    {
		float coef = *coefp;
		while (sndx--)
		    *out++ += *in++ * coef;
	    }
	    cellp++;
	    ovecp++;
	    gainp++;
	    coefp++;
	    incrp++;
	    bigincrp++;
	    nleftp++;
	}
    }
    osums = x->x_osums;
    indx = x->x_noutlets;
    while (indx--)
    {
	t_float *in = *osums++;
	t_float *out = *ovecs++;
	int sndx = nblock;
	while (sndx--)
	{
	    *out++ = *in;
	    *in++ = 0.;
	}
    }
    return (w + 3);
}

static void matrix_dsp(t_matrix *x, t_signal **sp)
{
    int i, nblock = sp[0]->s_n;
    t_float **vecp = x->x_ivecs;
    t_signal **sigp = sp;
    for (i = 0; i < x->x_ninlets; i++)
	*vecp++ = (*sigp++)->s_vec;
    vecp = x->x_ovecs;
    for (i = 0; i < x->x_noutlets; i++)
	*vecp++ = (*sigp++)->s_vec;
    if (nblock != x->x_nblock)
    {
	if (nblock > x->x_maxblock)
	{
	    size_t oldsize = x->x_maxblock * sizeof(*x->x_osums[i]),
		newsize = nblock * sizeof(*x->x_osums[i]);
	    for (i = 0; i < x->x_noutlets; i++)
		x->x_osums[i] = resizebytes(x->x_osums[i], oldsize, newsize);
	    x->x_maxblock = nblock;
	}
	x->x_nblock = nblock;
    }
    if (x->x_gains)
    {
	x->x_ksr = sp[0]->s_sr * .001;
	dsp_add(matrixnb_perform, 2, x, nblock);
    }
    else dsp_add(matrix01_perform, 2, x, nblock);
}

static void matrix_cellout(t_matrix *x, int indx, int ondx,
			   int onoff, float gain)
{
    t_atom atout[3];
    SETFLOAT(&atout[0], (t_float)indx);
    SETFLOAT(&atout[1], (t_float)ondx);
    if (onoff)
	SETFLOAT(&atout[2], gain);
    else
	SETFLOAT(&atout[2], 0.);
    outlet_list(x->x_dumpout, &s_list, 3, atout);
}

static void matrix_cellprint(t_matrix *x, int indx, int ondx,
			     int onoff, float gain)
{
    post("%d %d %g", indx, ondx, (onoff ? gain : 0.));
}

#ifdef MATRIX_DEBUG
static void matrix_celldebug(t_matrix *x, int indx, int ondx,
			     int onoff, float gain)
{
    loudbug_post("%d %d %g", indx, ondx, gain);
}
#endif

static void matrix_report(t_matrix *x, float *gains, float defgain,
			  t_matrix_cellfn cellfn)
{
    if (gains)
    {
	int *cellp = x->x_cells;
	float *gp = gains;
	int indx, ondx;
	for (indx = 0; indx < x->x_ninlets; indx++)
	    for (ondx = 0; ondx < x->x_noutlets; ondx++, cellp++, gp++)
		/* CHECKED all cells are printed */
		(*cellfn)(x, indx, ondx, *cellp, *gp);
    }
    else  /* CHECKED incompatible: gains confusingly printed in binary mode */
    {
	int *cellp = x->x_cells;
	int indx, ondx;
	for (indx = 0; indx < x->x_ninlets; indx++)
	    for (ondx = 0; ondx < x->x_noutlets; ondx++, cellp++)
		/* CHECKED all cells are printed */
		(*cellfn)(x, indx, ondx, *cellp, defgain);
    }
}

static void matrix_dump(t_matrix *x)
{
    matrix_report(x, x->x_coefs, 1., matrix_cellout);
}

static void matrix_dumptarget(t_matrix *x)
{
    matrix_report(x, x->x_gains, 1., matrix_cellout);
}

static void matrix_print(t_matrix *x)
{
    /* CHECKED same output as 'dump' -> [matrix~] -> [print] */
    matrix_report(x, x->x_coefs, 1., matrix_cellprint);
}

#ifdef MATRIX_DEBUG
static void matrix_debugramps(t_matrix *x)
{
    matrix_report(x, x->x_ramps, 0., matrix_celldebug);
}

static void matrix_debugsums(t_matrix *x)
{
    int i;
    loudbug_startpost("nblock %d (max %d), vectors:",
		      x->x_nblock, x->x_maxblock);
    for (i = 0; i < x->x_noutlets; i++)
	loudbug_startpost(" %x", (int)x->x_osums[i]);
    loudbug_endpost();
}

static void matrix_debug(t_matrix *x, t_symbol *s)
{
    if (s == gensym("ramps"))
	matrix_debugramps(x);
    else if (s == gensym("sums"))
	matrix_debugsums(x);
    else
    {
	matrix_debugramps(x);
	matrix_debugsums(x);
    }
}
#endif

static void matrix_free(t_matrix *x)
{
    if (x->x_ivecs)
	freebytes(x->x_ivecs, x->x_ninlets * sizeof(*x->x_ivecs));
    if (x->x_ovecs)
	freebytes(x->x_ovecs, x->x_noutlets * sizeof(*x->x_ovecs));
    if (x->x_osums)
    {
	int i;
	for (i = 0; i < x->x_noutlets; i++)
	    freebytes(x->x_osums[i], x->x_maxblock * sizeof(*x->x_osums[i]));
	freebytes(x->x_osums, x->x_noutlets * sizeof(*x->x_osums));
    }
    if (x->x_cells)
	freebytes(x->x_cells, x->x_ncells * sizeof(*x->x_cells));
    if (x->x_gains)
	freebytes(x->x_gains, x->x_ncells * sizeof(*x->x_gains));
    if (x->x_ramps)
	freebytes(x->x_ramps, x->x_ncells * sizeof(*x->x_ramps));
    if (x->x_coefs)
	freebytes(x->x_coefs, x->x_ncells * sizeof(*x->x_coefs));
    if (x->x_incrs)
	freebytes(x->x_incrs, x->x_ncells * sizeof(*x->x_incrs));
    if (x->x_bigincrs)
	freebytes(x->x_bigincrs, x->x_ncells * sizeof(*x->x_bigincrs));
    if (x->x_remains)
	freebytes(x->x_remains, x->x_ncells * sizeof(*x->x_remains));
}

static void *matrix_new(t_symbol *s, int ac, t_atom *av)
{
    t_pd *z;
    if (!fittermax_get() &&
	(z = fragile_class_mutate(matrixps_matrixtilde,
				  (t_newmethod)matrix_new, ac, av)))
	return (z);
    else if (ac < 2)
    {
	loud_error(0, "bad creation arguments for class '%s'",
		   matrixps_matrixtilde->s_name);
	loud_errand(0, "missing number of %s", (ac ? "outlets" : "inlets"));
	return (0);  /* CHECKED */
    }
    else
    {
	t_matrix *x = (t_matrix *)pd_new(matrix_class);
	int i;
	if (av[0].a_type == A_FLOAT)
	{
	    if ((x->x_ninlets = (int)av[0].a_w.w_float) < 1)
		x->x_ninlets = 1;
	}
	else x->x_ninlets = 1;  /* CHECKED */
	if (av[1].a_type == A_FLOAT)
	{
	    if ((x->x_noutlets = (int)av[1].a_w.w_float) < 1)
		x->x_noutlets = 1;
	}
	else x->x_noutlets = 1;  /* CHECKED */
	x->x_ncells = x->x_ninlets * x->x_noutlets;
	x->x_ivecs = getbytes(x->x_ninlets * sizeof(*x->x_ivecs));
	x->x_ovecs = getbytes(x->x_noutlets * sizeof(*x->x_ovecs));
	x->x_nblock = x->x_maxblock = sys_getblksize();
	x->x_osums = getbytes(x->x_noutlets * sizeof(*x->x_osums));
	for (i = 0; i < x->x_noutlets; i++)
	    x->x_osums[i] = getbytes(x->x_maxblock * sizeof(*x->x_osums[i]));
	x->x_cells = getbytes(x->x_ncells * sizeof(*x->x_cells));
	matrix_clear(x);
	if (ac >= 3)
	{
	    if (av[2].a_type == A_FLOAT)
		x->x_defgain = av[2].a_w.w_float;
	    else
		x->x_defgain = MATRIX_DEFGAIN;
	    x->x_gains = getbytes(x->x_ncells * sizeof(*x->x_gains));
	    for (i = 0; i < x->x_ncells; i++)
		x->x_gains[i] = x->x_defgain;
	    x->x_ramps = getbytes(x->x_ncells * sizeof(*x->x_ramps));
	    matrix_ramp(x, MATRIX_DEFRAMP);
	    x->x_coefs = getbytes(x->x_ncells * sizeof(*x->x_coefs));
	    for (i = 0; i < x->x_ncells; i++)
		x->x_coefs[i] = 0.;
	    x->x_ksr = sys_getsr() * .001;
	    x->x_incrs = getbytes(x->x_ncells * sizeof(*x->x_incrs));
	    x->x_bigincrs = getbytes(x->x_ncells * sizeof(*x->x_bigincrs));
	    x->x_remains = getbytes(x->x_ncells * sizeof(*x->x_remains));
	    for (i = 0; i < x->x_ncells; i++)
		x->x_remains[i] = 0;
	}
	else
	{
	    x->x_gains = 0;
	    x->x_ramps = 0;
	    x->x_coefs = 0;
	    x->x_incrs = 0;
	    x->x_bigincrs = 0;
	    x->x_remains = 0;
	}
	for (i = 1; i < x->x_ninlets; i++)
	    sic_newinlet((t_sic *)x, 0.);
	for (i = 0; i < x->x_noutlets; i++)
	    outlet_new((t_object *)x, &s_signal);
	x->x_dumpout = outlet_new((t_object *)x, &s_list);
	return (x);
    }
}

void matrix_tilde_setup(void)
{
    matrixps_matrixtilde = gensym("matrix~");
    matrix_class = class_new(matrixps_matrixtilde,
			     (t_newmethod)matrix_new,
			     (t_method)matrix_free,
			     sizeof(t_matrix), 0, A_GIMME, 0);
    fragile_class_raise(matrixps_matrixtilde, (t_newmethod)matrix_new);
    sic_setup(matrix_class, matrix_dsp, matrix_float);
    class_addlist(matrix_class, matrix_list);
    class_addmethod(matrix_class, (t_method)matrix_clear,
		    gensym("clear"), 0);
    class_addmethod(matrix_class, (t_method)matrix_set,
		    gensym("set"), A_FLOAT, A_DEFFLOAT, 0);
    class_addmethod(matrix_class, (t_method)matrix_connect,
		    gensym("connect"), A_GIMME, 0);
    class_addmethod(matrix_class, (t_method)matrix_connect,
		    gensym("disconnect"), A_GIMME, 0);
    class_addmethod(matrix_class, (t_method)matrix_ramp,
		    gensym("ramp"), A_FLOAT, 0);
    class_addmethod(matrix_class, (t_method)matrix_dump,
		    gensym("dump"), 0);
    class_addmethod(matrix_class, (t_method)matrix_dumptarget,
		    gensym("dumptarget"), 0);
    class_addmethod(matrix_class, (t_method)matrix_print,
		    gensym("print"), 0);
#ifdef MATRIX_DEBUG
    class_addmethod(matrix_class, (t_method)matrix_debug,
		    gensym("debug"), A_DEFSYM, 0);
#endif
    fitter_setup(matrix_class, 0);
}
