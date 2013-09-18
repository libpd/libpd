/* pipelist.c 20070711 Martin Peach based on pipe from x_time.c */
/* 20080706 added anything method for meta-messages */
#include "m_pd.h" 
/* -------------------------- pipe -------------------------- */

static t_class *pipelist_class;

typedef struct _hang
{
    t_clock             *h_clock;
    struct _hang        *h_next;
    struct _pipelist    *h_owner;
    int                 h_n; /* number of atoms in h_list */
    t_atom              *h_atoms; /* pointer to a list of h_n t_atoms */
} t_hang;

typedef struct _pipelist
{
    t_object    x_obj;
    float       x_deltime;
    t_outlet    *x_pipelistout;
    t_hang      *x_hang;
} t_pipelist;

static void *pipelist_new(t_symbol *s, int argc, t_atom *argv);
static void pipelist_hang_free(t_hang *h);
static void pipelist_hang_tick(t_hang *h);
static void pipelist_any_hang_tick(t_hang *h);
static void pipelist_list(t_pipelist *x, t_symbol *s, int ac, t_atom *av);
static void pipelist_anything(t_pipelist *x, t_symbol *s, int ac, t_atom *av);
static void pipelist_flush(t_pipelist *x);
static void pipelist_clear(t_pipelist *x);
void pipelist_setup(void);

static void *pipelist_new(t_symbol *s, int argc, t_atom *argv)
{
    t_pipelist  *x = (t_pipelist *)pd_new(pipelist_class);
    float       deltime;

    if (argc)
    { /* We accept one argument to set the delay time, ignore any further args */
        if (argv[0].a_type != A_FLOAT)
        {
            char stupid[80];
            atom_string(&argv[argc-1], stupid, 79);
            post("pipelist: %s: bad time delay value", stupid);
            deltime = 0;
        }
        else deltime = argv[argc-1].a_w.w_float;
    }
    else deltime = 0;

    x->x_pipelistout = outlet_new(&x->x_obj, &s_list);
    floatinlet_new(&x->x_obj, &x->x_deltime);
    x->x_hang = NULL;
    x->x_deltime = deltime;
    return (x);
}

static void pipelist_hang_free(t_hang *h)
{
    freebytes(h->h_atoms, h->h_n*sizeof(t_atom));
    clock_free(h->h_clock);
    freebytes(h, sizeof(t_hang));
}

static void pipelist_hang_tick(t_hang *h)
{
    t_pipelist  *x = h->h_owner;
    t_hang      *h2, *h3;

    /* excise h from the linked list of hangs */
    if (x->x_hang == h) x->x_hang = h->h_next;
    else for (h2 = x->x_hang; ((h3 = h2->h_next)!=NULL); h2 = h3)
    {
        if (h3 == h)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    /* output h's list */
    outlet_list(x->x_pipelistout, &s_list, h->h_n, h->h_atoms);
    /* free h */
    pipelist_hang_free(h);
}

static void pipelist_any_hang_tick(t_hang *h)
{
    t_pipelist  *x = h->h_owner;
    t_hang      *h2, *h3;

    /* excise h from the linked list of hangs */
    if (x->x_hang == h) x->x_hang = h->h_next;
    else for (h2 = x->x_hang; ((h3 = h2->h_next)!=NULL); h2 = h3)
    {
        if (h3 == h)
        {
            h2->h_next = h3->h_next;
            break;
        }
    }
    /* output h's atoms */
    outlet_anything(x->x_pipelistout, h->h_atoms[0].a_w.w_symbol, h->h_n-1, &h->h_atoms[1]);
    /* free h */
    pipelist_hang_free(h);
}

static void pipelist_list(t_pipelist *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_deltime > 0)
    { /* if delay is real, save the list for output in delay milliseconds */
        t_hang *h;
        int i;

        h = (t_hang *)getbytes(sizeof(t_hang));
        h->h_n = ac;
        h->h_atoms = (t_atom *)getbytes(h->h_n*sizeof(t_atom));

        for (i = 0; i < h->h_n; ++i)
            h->h_atoms[i] = av[i];
        h->h_next = x->x_hang;
        x->x_hang = h;
        h->h_owner = x;
        h->h_clock = clock_new(h, (t_method)pipelist_hang_tick);
        clock_delay(h->h_clock, (x->x_deltime >= 0 ? x->x_deltime : 0));
    }
    /* otherwise just pass the list straight through  */
    else outlet_list(x->x_pipelistout, &s_list, ac, av);
}

static void pipelist_anything(t_pipelist *x, t_symbol *s, int ac, t_atom *av)
{
    if (x->x_deltime > 0)
    { /* if delay is real, save the list for output in delay milliseconds */
        t_hang *h;
        int i;

        h = (t_hang *)getbytes(sizeof(t_hang));
        h->h_n = ac+1;
        h->h_atoms = (t_atom *)getbytes(h->h_n*sizeof(t_atom));
        SETSYMBOL(&h->h_atoms[0], s);
        for (i = 1; i < h->h_n; ++i)
            h->h_atoms[i] = av[i-1];
        h->h_next = x->x_hang;
        x->x_hang = h;
        h->h_owner = x;
        h->h_clock = clock_new(h, (t_method)pipelist_any_hang_tick);
        clock_delay(h->h_clock, (x->x_deltime >= 0 ? x->x_deltime : 0));
    }
    /* otherwise just pass it straight through  */
    else outlet_anything(x->x_pipelistout, s, ac, av);
}

static void pipelist_flush(t_pipelist *x)
{
    while (x->x_hang) pipelist_hang_tick(x->x_hang);
}

static void pipelist_clear(t_pipelist *x)
{
    t_hang *hang;
    while ((hang = x->x_hang) != NULL)
    {
        x->x_hang = hang->h_next;
        pipelist_hang_free(hang);
    }
}

void pipelist_setup(void)
{
    pipelist_class = class_new(gensym("pipelist"), 
        (t_newmethod)pipelist_new, (t_method)pipelist_clear,
        sizeof(t_pipelist), 0, A_GIMME, 0);
    class_addlist(pipelist_class, pipelist_list);
    class_addanything(pipelist_class, pipelist_anything);
    class_addmethod(pipelist_class, (t_method)pipelist_flush, gensym("flush"), 0);
    class_addmethod(pipelist_class, (t_method)pipelist_clear, gensym("clear"), 0);
}

/* end of pipelist.c*/
