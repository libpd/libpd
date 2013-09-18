/* Life2x.c -- The Game of Life (2D Cellular Automaton) ------- */
/* © Bill Vorn 2002 */
/* modified by Martin Peach September 2002 */
/* implemented grey scale display*/
/* change gen_ arrays to pointers for better memory peformance */
/* <<better memory!>> */
/* MP20060517 Windows version */
/* MP 20080819 pd version with no graphics */

#include "m_pd.h"
#include <stdio.h> /* for sprintf() */
#include <stdlib.h> /* for random() */
#include <time.h> /* for clock() */

#ifdef _WIN32 /* Windows doesn't have these named correctly ;) */
# define random rand
# define srandom srand
#endif

#define MAXSIZE 1024
#define DEFAULT_DIM 16

static t_class *life2x_class;

typedef struct life2x
{
    t_object    x_obj;
    char        *gen_origin_ptr; /* [MAXSIZE]X[MAXSIZE]; */
    char        *gen_finale_ptr; /* [MAXSIZE][MAXSIZE]; */
    char        *gen_start_ptr; /* [MAXSIZE][MAXSIZE]; */
    char        *gen_shift_ptr; /* [MAXSIZE][MAXSIZE]; */
    t_atom      *l_column_list; /* values for one column */
    long        l_cellmax; /* number of bytes in gen */
    long        l_xcellnum;
    long        l_ycellnum;
    long        l_gennum;
    long        l_livenum;
    long        l_deltanum;
    long        l_lastdeltanum;
    long        l_novarinum;
    long        l_novar;
    short       l_deadflag;
    short       l_thruflag;
    short       l_xshift;
    short       l_yshift;
    short       l_invertflag;
    t_outlet    *l_genout;
    t_outlet    *l_liveout;
    t_outlet    *l_deltaout;
    t_outlet    **l_cellouts;
    t_outlet    *l_novariout;
    t_outlet    *l_deadout;
    t_outlet    *l_dumpout;
    char        l_survive[9]; /* rule for survival according to neighbour count */
    char        l_born[9]; /* rule for birth according to neighbour count */
} t_life2x;

static void life2x_bang(t_life2x *x);
static void life2x_output_cells(t_life2x *x);
static void life2x_set(t_life2x *x, t_floatarg xx, t_floatarg yy, t_floatarg state);
static void life2x_clear(t_life2x *x);
static void life2x_reset(t_life2x *x);
static void life2x_return(t_life2x *x);
static void life2x_dump(t_life2x *x);
static void life2x_rule(t_life2x *x, t_symbol *s);
static void life2x_randomize(t_life2x *x, t_floatarg f);
static void life2x_thru(t_life2x *x, t_floatarg f);
static void life2x_shift(t_life2x *x, t_floatarg f1, t_floatarg f2);
static void life2x_flipv(t_life2x *x);
static void life2x_fliph(t_life2x *x);
static void life2x_invert(t_life2x *x, t_floatarg f);
static void life2x_novar(t_life2x *x, t_floatarg f);
static void life2x_free(t_life2x *x);
static void *life2x_new(t_symbol *s, short ac, t_atom *av);

void life2x_setup(void)
{
    life2x_class = class_new(gensym("life2x"),
        (t_newmethod)life2x_new,
        (t_method)life2x_free,
        sizeof(t_life2x),
        CLASS_DEFAULT,
        A_GIMME,
        0);
    class_addbang(life2x_class, life2x_bang);
    class_addmethod(life2x_class, (t_method)life2x_clear, gensym("clear"), 0);
    class_addmethod(life2x_class, (t_method)life2x_reset, gensym("reset"), 0);
    class_addmethod(life2x_class, (t_method)life2x_return, gensym("return"), 0);
    class_addmethod(life2x_class, (t_method)life2x_dump, gensym("dump"), 0);
    class_addmethod(life2x_class, (t_method)life2x_rule, gensym("rule") ,A_DEFSYMBOL, 0);
    class_addmethod(life2x_class, (t_method)life2x_randomize, gensym("randomize"), A_DEFFLOAT, 0);
    class_addmethod(life2x_class, (t_method)life2x_thru, gensym("thru") ,A_DEFFLOAT, 0);
    class_addmethod(life2x_class, (t_method)life2x_shift, gensym("shift"), A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(life2x_class, (t_method)life2x_set, gensym("set"), A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
    class_addmethod(life2x_class, (t_method)life2x_flipv, gensym("flipv"), 0);
    class_addmethod(life2x_class, (t_method)life2x_fliph, gensym("fliph"), 0);
    class_addmethod(life2x_class, (t_method)life2x_invert, gensym("invert"), A_DEFFLOAT, 0);
    class_addmethod(life2x_class, (t_method)life2x_novar, gensym("novar"), A_DEFFLOAT, 0);
    return;
}

static void life2x_bang(t_life2x *x)
{
    short           i, j, m, n, p, q;
    short           i_pre, i_post, j_pre, j_post;
    short           xmax, ymax;
    unsigned long   cellmax;
    short           xshft, yshft;
    char            c;
    short           cellval;
    long            gendiff, lastgendiff;
    char            *g_start, *g_origin, *g_final, *g_shift;
    int             c_live;

    xmax = x->l_xcellnum - 1;
    ymax = x->l_ycellnum - 1;
    cellmax = x->l_xcellnum * x->l_ycellnum;
    xshft = x->l_xshift;
    yshft = x->l_yshift;

    if (x->l_gennum == 0)
    {
        g_start = x->gen_start_ptr;
        g_origin = x->gen_origin_ptr;
        for (j = 0; j < cellmax; ++j) *g_start++ = *g_origin++;
    }

    for (j = 0; j <= ymax; j++)
    {
        j_pre = j-1;
        if (j_pre < 0) j_pre = ymax;
        j_post = j+1;
        if (j_post > ymax) j_post = 0;
        for (i = 0; i <= xmax; i++)
        {
            i_pre = i-1;
            if (i_pre < 0) i_pre = xmax;
            i_post = i+1;
            if (i_post > xmax) i_post = 0;
             /* check each neighbour */
            cellval = (*(x->gen_origin_ptr+(j_pre*x->l_xcellnum)+(i_pre)) != 0)
                        +(*(x->gen_origin_ptr+(j_pre*x->l_xcellnum)+i) != 0)
                        +(*(x->gen_origin_ptr+(j_pre*x->l_xcellnum)+i_post) != 0)
                        +(*(x->gen_origin_ptr+(j*x->l_xcellnum)+i_pre) != 0)
                        +(*(x->gen_origin_ptr+(j*x->l_xcellnum)+i_post) != 0)
                        +(*(x->gen_origin_ptr+(j_post*x->l_xcellnum)+i_pre) != 0)
                        +(*(x->gen_origin_ptr+(j_post*x->l_xcellnum)+i) != 0)
                        +(*(x->gen_origin_ptr+(j_post*x->l_xcellnum)+i_post) != 0);

            c = *(x->gen_origin_ptr+(j*x->l_xcellnum)+i); /* current value of cell */
            g_final = (x->gen_finale_ptr+(j*x->l_xcellnum)+i);

            /* apply the rule */
            if (c == 0) c_live = x->l_born[cellval]; /* cell is born if it has the right number of neighbours */
            else c_live = x->l_survive[cellval]; /* cell survives if it has the right number of neighbours */

            if (c_live == 1)
            {
                if (c < 8) *g_final = c + 1;
                else *g_final = 8;
            }
            else *g_final = 0;
        }
    }

    if ((xshft != 0) || (yshft != 0))
    { /* if there is a shift offset */
        for (j = 0; j <= ymax; j++)
        {
            n = j + yshft;
            if (n < 0) q = (n + ymax) + 1;
            else if (n > ymax) q = (n - ymax) - 1;
            else q = n;
            for (i = 0; i <= xmax; i++)
            {
                m = i + xshft;
                if (m < 0) p = (m + xmax) + 1;
                else if (m > xmax) p = (m - xmax) - 1;
                else p = m;
                *(x->gen_shift_ptr+(q * x->l_xcellnum)+p) = *(x->gen_finale_ptr+(j * x->l_xcellnum)+i);
            }
        }
        g_final = x->gen_finale_ptr;
        g_shift = x->gen_shift_ptr;

        for (j = 0; j <= cellmax; j++) *g_final++ = *g_shift++;
    }

    x->l_gennum = x->l_gennum + 1; /* increment generation # */
    outlet_float(x->l_genout, x->l_gennum); /* output generation # */

    life2x_output_cells(x);

    gendiff = x->l_livenum - x->l_deltanum; /* compare # of live cells from last gen to current gen */
    outlet_float(x->l_deltaout, gendiff); /* output delta # of live cells from last gen to current gen */
    x->l_deltanum = x->l_livenum; /* store # of live cells from current gen */

    outlet_float(x->l_liveout, x->l_livenum); /* output # of live cells in current generation */

    lastgendiff = x->l_lastdeltanum;
    if (gendiff == lastgendiff) x->l_novar--;
    else x->l_novar = x->l_novarinum; /* reset countdown */
    if (x->l_novar == 0)
    {
        outlet_bang(x->l_novariout);
        x->l_novar = x->l_novarinum;
    }
    x->l_lastdeltanum = gendiff;

    x->l_deadflag = 0;
    for (j = 0; j < x->l_ycellnum; j++)
    {
        for (i = 0; i < x->l_xcellnum; i++)
        {
            g_final = (x->gen_finale_ptr+(j*x->l_xcellnum)+i);
            g_origin = (x->gen_origin_ptr+(j*x->l_xcellnum)+i);
            /* check for gen difference: */
            if (*g_final != *g_origin) x->l_deadflag = 1;
            *g_origin = *g_final; /* this generation becomes last generation */
        }
    }
    /* check for dead world: */
    if (x->l_deadflag == 0) outlet_bang(x->l_deadout);

    return;
}

static void life2x_output_cells(t_life2x *x)
{
    char    *g_final;
    short   i, j;
    long    d = 0;

    for (i = 0; i < x->l_xcellnum; i++)
    { /* output all cells state for each column */
        for (j = 0; j < x->l_ycellnum; j++)
        {
            g_final = (x->gen_finale_ptr+(j*x->l_xcellnum)+i);
            if (x->l_invertflag != 0) x->l_column_list[j].a_w.w_float = (10 - *g_final)%9;
            else x->l_column_list[j].a_w.w_float = *g_final;
            if (*g_final != 0) d++;
        }
        outlet_list(x->l_cellouts[x->l_xcellnum-1-i], &s_list, j, x->l_column_list);
    }
    x->l_livenum = d;

    return;
}

static void life2x_set(t_life2x *x, t_floatarg xx, t_floatarg yy, t_floatarg state)
{ /* a list of three floats to set the state of cell (xx,yy) */
    short   i, j, k;

    if (xx < 0) i = 0;
    if (xx >= x->l_xcellnum) i = x->l_xcellnum - 1;
    else i = (short)xx;
    if (yy < 0) j = 0;
    if (yy > x->l_ycellnum) j = x->l_ycellnum - 1;
    else j = (short)yy;

    if (state < 0) k = 0;
    if (state > 8) k = 8;
    else k = (short)state;

    *(x->gen_origin_ptr+(j*x->l_xcellnum+i)) = k;
    *(x->gen_finale_ptr+(j*x->l_xcellnum+i)) = k;

    if (x->l_thruflag != 0) life2x_output_cells(x);
    return;
}

static void life2x_clear(t_life2x *x)
{
    short   j;
    char    *g_origin, * g_final;

    g_origin = x->gen_origin_ptr;
    g_final = x->gen_finale_ptr;

    for (j = 0; j < x->l_cellmax; ++j) *g_origin++ = *g_final++ = 0;

    if (x->l_thruflag != 0) life2x_output_cells(x);

    return;
}

static void life2x_reset(t_life2x *x)
{
    short   j;
    char    *g_origin, * g_final;

    g_origin = x->gen_origin_ptr;
    g_final = x->gen_finale_ptr;
    x->l_gennum = 0;

    outlet_float (x->l_genout, x->l_gennum); /* output generation 0 */

    for (j = 0; j < x->l_cellmax; ++j) *g_origin++ = *g_final++ = 0;

    x->l_xshift = 0;
    x->l_yshift = 0;
    x->l_deltanum = 0;
    x->l_novar = x->l_novarinum;

    if (x->l_thruflag != 0) life2x_output_cells(x);

    return;
}

static void life2x_return(t_life2x *x)
{
    short   i, j;
    char    *g_origin, *g_start, *g_final;

    x->l_gennum = 0;
    outlet_float (x->l_genout, x->l_gennum); /* output generation 0 */

    for (j = 0; j < x->l_ycellnum; j++)
    {
        for (i = 0; i < x->l_xcellnum; i++)
        {
            g_origin = (x->gen_origin_ptr+(j*x->l_xcellnum)+i);
            g_start = (x->gen_start_ptr+(j*x->l_xcellnum)+i);
            g_final = (x->gen_finale_ptr+(j*x->l_xcellnum)+i);

            *g_origin = *g_start;
            *g_final = *g_start;
        }
    }
    if (x->l_thruflag != 0) life2x_output_cells(x);
    return;
}

static void life2x_dump(t_life2x *x)
{
    short           i, j, k;
    unsigned long   count = 0;
    unsigned long   outVal[3];
    t_atom          outList[3];
    char            *g_final = x->gen_finale_ptr;

    for (j = 0; j < x->l_ycellnum; ++j)
    {
        for (i = 0; i < x->l_xcellnum; ++i)
        {
            if (*g_final++)
            {
                outVal[0] = count;
                outVal[1] = i;
                outVal[2] = j;
                for (k = 0; k < 3; k++) SETFLOAT(&outList[k], outVal[k]);
                outlet_list (x->l_dumpout, &s_list, 3, outList);
                count++;
            }
        }
    }
    return;
}

static void life2x_rule(t_life2x *x, t_symbol *s)
{
    short   i;
    char    survive[9];
/* one entry for each possible neighbour count, set to one if cell survives with that many neighbours */
    char    born[9];
/* one entry for each possible neighbour count, set to one if cell is born with that many neighbours */

    for (i = 0; i < 9; ++i) survive[i] = born[i] = 0;

    for (i = 0; s->s_name[i] != 0; ++i)
    {
        if (s->s_name[i] == '/') break;
        if ((s->s_name[i] < 0x30)||(s->s_name[i] > 0x38))
        {
            error("life2x_rule: bad character in rule: %c", s->s_name[i]);
            return;
        }
        survive[s->s_name[i]-0x30] = 1;
    }
    if (s->s_name[i] != '/')
    {
        error("life2x_rule: missing / separator");
        return;
    }
    for (++i; s->s_name[i] != 0; ++i)
    {
        if ((s->s_name[i] < 0x30)||(s->s_name[i] > 0x38))
        {
            error("life2x_rule: bad character in rule: %c", s->s_name[i]);
            return;
        }
        born[s->s_name[i]-0x30] = 1;
    }
    for (i = 0; i < 9; ++i)
    { /* update the rule */
        x->l_survive[i] = survive[i];
        x->l_born[i] = born[i];
    }
    return;
}

static void life2x_randomize(t_life2x *x, t_floatarg f)
{ /* set a random fraction of the array alive */
    short   i, j;
    float   threshold;

    if (f > 1.0) f = 1.0;
    else if (f < 0.0) f = 0.0;
    threshold = RAND_MAX*f; /* RAND_MAX is 0x7FFFFFFF on linux */

    for (j = 0; j < x->l_ycellnum; j++)
    {
        for (i = 0; i < x->l_xcellnum; i++)
        {
            if ((random() < threshold))
            { 
                *(x->gen_origin_ptr+(j*x->l_xcellnum)+i) = 1;
                *(x->gen_finale_ptr+(j*x->l_xcellnum)+i) = 1;
            }
        }
    }
    if (x->l_thruflag != 0) life2x_output_cells(x);
    return;
}

static void life2x_thru(t_life2x *x, t_floatarg f)
{
    long n = (long)f;

    x->l_thruflag = (n == 0)? 0: 1;
    return;
}

static void life2x_shift(t_life2x *x, t_floatarg f1, t_floatarg f2)
{
    long n = (long)f1;

    n %= x->l_xcellnum;
    x->l_xshift = n;
    n = (long)f2;
    n %= x->l_ycellnum;
    x->l_yshift = n;
    return;
}

static void life2x_flipv(t_life2x *x)
{
    short   a, i, j;

    for (i = 0; i < x->l_xcellnum; ++i)
    {
        a = x->l_xcellnum - 1;
        for (j = 0; j < x->l_xcellnum; ++j)
        {
            *(x->gen_shift_ptr+(a*x->l_xcellnum)+i) = *(x->gen_finale_ptr+(j*x->l_xcellnum)+i);
            a--;
        }
    }
    for (j = 0; j < x->l_ycellnum; ++j)
    {
        for (i = 0; i < x->l_xcellnum; ++i)
        {
            *(x->gen_finale_ptr+(j*x->l_xcellnum)+i) = *(x->gen_shift_ptr+(j*x->l_xcellnum)+i);
            *(x->gen_origin_ptr+(j*x->l_xcellnum)+i) = *(x->gen_shift_ptr+(j*x->l_xcellnum)+i);
        }
    }

    if (x->l_thruflag != 0) life2x_output_cells(x);

    return;
}

static void life2x_fliph(t_life2x *x)
{
    short   a, i, j;

    for (j = 0; j < x->l_ycellnum - 1; ++j)
    {
        a = x->l_xcellnum - 1;
        for (i = 0; i < x->l_xcellnum; ++i)
        {
            *(x->gen_shift_ptr+(j*x->l_xcellnum)+a) = *(x->gen_finale_ptr+(j*x->l_xcellnum)+i);
            a--;
        }
    }
    for (j = 0; j < x->l_ycellnum - 1; ++j)
    {
        for (i = 0; i < x->l_xcellnum - 1; ++i)
        {
            *(x->gen_finale_ptr+(j*x->l_xcellnum)+i) = *(x->gen_shift_ptr+(j*x->l_xcellnum)+i);
            *(x->gen_origin_ptr+(j*x->l_xcellnum)+i) = *(x->gen_shift_ptr+(j*x->l_xcellnum)+i);
        }
    }

    if (x->l_thruflag != 0) life2x_output_cells(x);

    return;
}

static void life2x_invert(t_life2x *x, t_floatarg f)
{
    x->l_invertflag = (f == 0)? 0: 1;

    if (x->l_thruflag != 0) life2x_output_cells(x);

    return;
}

static void life2x_novar(t_life2x *x, t_floatarg f)
{
    long n = (long)f;

    if (n < 0)
    {
        error("life2x: novar argument must be positive");
        return;
    }
    else
    {
        x->l_novarinum = n;
        x->l_novar = n;
    }
    return;
}


static void life2x_free(t_life2x *x)
{
    if (x->gen_origin_ptr != NULL) freebytes (x->gen_origin_ptr, x->l_cellmax);
    if (x->gen_finale_ptr != NULL) freebytes (x->gen_finale_ptr, x->l_cellmax);
    if (x->gen_start_ptr != NULL) freebytes (x->gen_start_ptr, x->l_cellmax);
    if (x->gen_shift_ptr != NULL) freebytes (x->gen_shift_ptr, x->l_cellmax);
    if (x->l_column_list != NULL) freebytes(x->l_column_list, x->l_ycellnum*sizeof (t_atom));
    if (x->l_cellouts != NULL) freebytes(x->l_cellouts, x->l_xcellnum*sizeof (t_outlet*));
    return;
}


static void *life2x_new(t_symbol *s, short ac, t_atom *av)
{
    t_life2x    *x;
    short       i;
    short       j;
    short       xmax, ymax;

    x = (t_life2x *)pd_new(life2x_class);

    if (ac > 0)
    { /* if there is at least 1 argument */
        if (av[0].a_type == A_FLOAT)
        { /* if first arg is an int */
            x->l_xcellnum = av[0].a_w.w_float; /* arg sets # of horiz cells */
            if (x->l_xcellnum < 4)
            {
                error("Life: first argument < 4, set to 4");
                x->l_xcellnum = 4; /* min # of  horiz cells */
            }
            if (x->l_xcellnum > MAXSIZE)
            {
                error("Life: first argument > %d, set to %d", MAXSIZE, MAXSIZE);
                x->l_xcellnum = MAXSIZE; /* max # of horiz cells */
            }
            /* if 2 arguments */
            if ((ac > 1) && (av[1].a_type == A_FLOAT))
            { /* if 2nd arg is an int */
                x->l_ycellnum = av[1].a_w.w_float; /* 2nd arg sets # of verti cells */
                if (x->l_ycellnum < 4)
                {
                    error("Life: 2nd argument < 4, set to 4");
                    x->l_ycellnum = 4; /* min # of verti cells */
                }
                if (x->l_ycellnum > MAXSIZE)
                {
                    error("Life: 2nd argument > %d, set to %d", MAXSIZE, MAXSIZE);
                    x->l_ycellnum = MAXSIZE; /* max # of verti cells */
                }
            }
            else if (ac > 1)
            { /* if 2nd arg not an int */
                error("Life: 2nd argument must be int");
                x->l_ycellnum = DEFAULT_DIM; /* default # of verti cells */
            }
            else
            { /* if no 2nd arg */
                x->l_ycellnum = x->l_xcellnum; /* # of verti cells = # of horiz cells */
            }
        }
        else
        { /*if first arg not an int */
            error("Life: first argument must be int");
            x->l_xcellnum = DEFAULT_DIM; /* default # of horiz cells */
            x->l_ycellnum = DEFAULT_DIM; /* default # of verti cells */
        }
    }
    else
    { /* if no arg */
        x->l_xcellnum = DEFAULT_DIM; /* default # of horiz cells */
        x->l_ycellnum = DEFAULT_DIM; /* default # of verti cells */
    }

    x->l_cellmax = x->l_xcellnum*x->l_ycellnum;
    if ((x->gen_origin_ptr = getbytes(x->l_cellmax)) != NULL)
        if ((x->gen_finale_ptr = getbytes(x->l_cellmax)) != NULL)
            if ((x->gen_start_ptr = getbytes(x->l_cellmax)) != NULL)
               x->gen_shift_ptr = getbytes(x->l_cellmax);
    if
    (
        (x->gen_origin_ptr == NULL)
        || (x->gen_finale_ptr == NULL)
        || (x->gen_start_ptr == NULL)
        || (x->gen_shift_ptr == NULL)
    )
    {
        error ("Unable to allocate memory for the life array (%luX%lu needs %lu bytes)",
            x->l_xcellnum, x->l_ycellnum, x->l_cellmax*4L);
        life2x_free (x);
        return x;
    }

    x->l_column_list = getbytes(x->l_ycellnum*sizeof (t_atom));
    if (x->l_column_list == NULL)
    {
        error("life2x_new: Unable to allocate %lu bytes for column list",
        x->l_ycellnum*sizeof (t_atom));
        life2x_free (x);
        return x;
    }
    for (j = 0; j < x->l_ycellnum; ++j) SETFLOAT(&x->l_column_list[j], 0);
    /* (we can go faster later by making the atoms floats now) */

    xmax = x->l_xcellnum - 1;
    ymax = x->l_ycellnum - 1;

    post("life new...(%d X %d)", x->l_xcellnum, x->l_ycellnum);
    for (j = 0; j <= ymax; j++)
    { /* sets all cells to 0 */
        for (i = 0; i <= xmax; i++)
        {
            *(x->gen_origin_ptr+(j*x->l_xcellnum)+i) = 0;
            *(x->gen_finale_ptr+(j*x->l_xcellnum)+i) = 0;
            *(x->gen_start_ptr+(j*x->l_xcellnum)+i) = 0;
            *(x->gen_shift_ptr+(j*x->l_xcellnum)+i) = 0;
        }
    }

    x->l_xshift = 0;
    x->l_yshift = 0;
    x->l_deltanum = 0;
    x->l_lastdeltanum = 0;
    x->l_novarinum = 32;
    x->l_novar = 32;

    x->l_cellouts = getbytes(x->l_xcellnum*sizeof (t_outlet*));
    if (x->l_cellouts == NULL)
    {
        error("life2x_new: Unable to allocate %lu bytes for column outlets",
        x->l_xcellnum*sizeof (t_outlet*));
        life2x_free (x);
        return x;
    }

    for (i = xmax; i >= 0; i--) x->l_cellouts[i] = outlet_new(&x->x_obj, &s_list);/* create an outlet for each column */
    x->l_deltaout = outlet_new(&x->x_obj, &s_float); /* create outlet for live cell diff from previous gen */
    x->l_liveout = outlet_new(&x->x_obj, &s_float); /* sets outlet for # of live cells in current generation */
    x->l_genout = outlet_new(&x->x_obj, &s_float); /* sets outlet for current generation # */
    x->l_dumpout = outlet_new(&x->x_obj, &s_list); /* sets outlet for list dumping */
    x->l_novariout = outlet_new(&x->x_obj, &s_bang); /* sets outlet for no variation period bang */
    x->l_deadout = outlet_new(&x->x_obj, &s_bang); /* sets first outlet for dead world bang */
    x->l_gennum = 0; /* sets first generation # */
    outlet_float(x->l_genout, x->l_gennum);
    x->l_thruflag = 1; /* sets thru mode on */
    x->l_invertflag = 0; /* sets invert off */

    /* set up default Conway rule */
    life2x_rule(x, gensym("23/3")); /* survive if 2 or 3 neighbours / born if 3 neighbours */
    srandom(clock()); /* seed the random number generator */
    return (x);
}

/* end of life2x.c */
