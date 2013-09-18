/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "unstable/fragile.h"

#ifdef MSW
#define snprintf  _snprintf
#endif

#define TESTMESS_INISIZE      4  /* LATER rethink */
#define TESTMESS_STACKSIZE  256

typedef struct _testmess
{
    t_object     x_ob;
    t_symbol    *x_method;
    void       (*x_messfun)(struct _testmess *, t_symbol *s, int, t_atom *);
    int          x_appendmode;
    int          x_size;      /* as allocated */
    int          x_natoms;    /* as used */
    int          x_tailwise;  /* data is moved to the end of a buffer */
    t_atom      *x_message;
    t_atom      *x_messbuf;
    t_atom       x_messini[TESTMESS_INISIZE];
} t_testmess;

static t_class *testmess_class;

static void testmess_setnatoms(t_testmess *x, int natoms)
{
    if (x->x_tailwise)
	x->x_message = x->x_messbuf + x->x_size - natoms;
    else
	x->x_message = x->x_messbuf;
    x->x_natoms = natoms;
}

static int testmess_makeroom(t_testmess *x, int natoms, int preserve)
{
    if (x->x_size < natoms)
    {
	int newsize = x->x_size * 2;
	while (newsize < natoms) newsize *= 2;
/*	post("makeroom %s %d %d %d", x->x_method->s_name,
	     preserve, natoms, newsize);*/
	if (x->x_messbuf == x->x_messini)
	{
	    if (!(x->x_messbuf =
		  (t_atom *)getbytes(newsize * sizeof(*x->x_messbuf))))
	    {
		x->x_messbuf = x->x_messini;
		testmess_setnatoms(x, preserve ? x->x_natoms : 0);
		return (0);
	    }
	    x->x_size = newsize;
	    testmess_setnatoms(x, preserve ? x->x_natoms : 0);
	    if (x->x_natoms)
	    {
		if (x->x_tailwise)
		    memcpy(x->x_message,
			   x->x_messini + TESTMESS_INISIZE - x->x_natoms,
			   x->x_natoms * sizeof(*x->x_message));
		else
		    memcpy(x->x_message,
			   x->x_messini, x->x_natoms * sizeof(*x->x_message));
	    }
	}
	else
	{
	    int oldsize = x->x_size;
	    if (!(x->x_messbuf =
		  (t_atom *)resizebytes(x->x_messbuf,
					x->x_size * sizeof(*x->x_messbuf),
					newsize * sizeof(*x->x_messbuf))))
	    {
		x->x_messbuf = x->x_messini;
		x->x_size = TESTMESS_INISIZE;
		testmess_setnatoms(x, 0);
		return (0);
	    }
	    x->x_size = newsize;
	    testmess_setnatoms(x, preserve ? x->x_natoms : 0);
	    if (x->x_natoms && x->x_tailwise)
		memmove(x->x_message, x->x_messbuf + oldsize - x->x_natoms,
			x->x_natoms * sizeof(*x->x_message));
	}
    }
    return (1);
}

static void testmess_stackmess(t_testmess *x, t_symbol *s, int ac, t_atom *av)
{
    t_atom buf[TESTMESS_STACKSIZE];
    int natoms = x->x_natoms;
    if (x->x_appendmode)
    {
	int left = TESTMESS_STACKSIZE - ac;
	if (left < 0) ac = TESTMESS_STACKSIZE, natoms = 0;
	else if (natoms > left) natoms = left;
	if (ac)
	    memcpy(buf, av, ac * sizeof(*buf));
	if (natoms)
	    memcpy(buf + ac, x->x_message, natoms * sizeof(*buf));
    }
    else
    {
	int left = TESTMESS_STACKSIZE - natoms;
	if (left < 0) natoms = TESTMESS_STACKSIZE, ac = 0;
	else if (ac > left) ac = left;
	if (natoms)
	    memcpy(buf, x->x_message, natoms * sizeof(*buf));
	if (ac)
	    memcpy(buf + natoms, av, ac * sizeof(*buf));
    }
    outlet_anything(((t_object *)x)->ob_outlet, s, natoms + ac, buf);
}

static void testmess_heapmess(t_testmess *x, t_symbol *s, int ac, t_atom *av)
{
    int ntotal = x->x_natoms + ac;
    t_atom *buf = getbytes(ntotal * sizeof(*buf));
    if (buf)
    {
	if (x->x_appendmode)
	{
	    if (ac)
		memcpy(buf, av, ac * sizeof(*buf));
	    if (x->x_natoms)
		memcpy(buf + ac, x->x_message, x->x_natoms * sizeof(*buf));
	}
	else
	{
	    if (x->x_natoms)
		memcpy(buf, x->x_message, x->x_natoms * sizeof(*buf));
	    if (ac)
		memcpy(buf + x->x_natoms, av, ac * sizeof(*buf));
	}
	outlet_anything(((t_object *)x)->ob_outlet, s, ntotal, buf);
	freebytes(buf, ntotal * sizeof(*buf));
    }
}

static void testmess_premess(t_testmess *x, t_symbol *s, int ac, t_atom *av)
{
    int ntotal = x->x_natoms + ac;
    if (testmess_makeroom(x, ntotal, 1))
    {
	t_atom *buf;
	if (x->x_appendmode)
	{
	    buf = x->x_messbuf + x->x_size - ntotal;
	    if (ac)
		memcpy(buf, av, ac * sizeof(*buf));
	}
	else
	{
	    buf = x->x_messbuf;
	    if (ac)
		memcpy(buf + x->x_natoms, av, ac * sizeof(*buf));
	}
	outlet_anything(((t_object *)x)->ob_outlet, s, ntotal, buf);
    }
}

static void testmess_bang(t_testmess *x)
{
    if (x->x_natoms)
	x->x_messfun(x, &s_list, 0, 0);
}

static void testmess_float(t_testmess *x, t_float f)
{
    t_atom at;
    SETFLOAT(&at, f);
    x->x_messfun(x, (x->x_natoms ? &s_list : &s_float), 1, &at);
}

static void testmess_symbol(t_testmess *x, t_symbol *s)
{
    x->x_messfun(x, s, 0, 0);
}

static void testmess_anything(t_testmess *x, t_symbol *s, int ac, t_atom *av)
{
    x->x_messfun(x, s, ac, av);
}

static void testmess_setnumbers(t_testmess *x, int natoms, int start)
{
    if (natoms <= 0)
	natoms = 100;
    if (testmess_makeroom(x, natoms * 2, 0))
    {
	t_atom *ap;
	testmess_setnatoms(x, natoms);
	ap = x->x_message;
	while (natoms--)
	{
	    SETFLOAT(ap, (t_float)start);
	    start++; ap++;
	}
    }
}

#define FRAGILE_HASHSIZE  1024

static int fragile_hash(t_symbol *s)
{
    unsigned int hash1 = 0,  hash2 = 0;
    char *ptr = s->s_name;
    while (*ptr)
    {
        hash1 += *ptr++;
        hash2 += hash1;
    }
    return (hash2 & (FRAGILE_HASHSIZE-1));
}

int fragile_symbol_count(void)
{
    return (100);
}

void fragile_getsymbols(t_atom *av)
{
    t_symbol *s = gensym("#N");
    int i;
    for (i = 0, s -= fragile_hash(s); i < FRAGILE_HASHSIZE; i++, s++)
    {
	if (s->s_name)
	{
	    t_symbol *s1;
	    for (s1 = s; s1; s1 = s1->s_next)
		printf("%s\n", s1->s_name);
	}
    }
}

static void testmess_setnames(t_testmess *x, t_symbol *s,
			      int natoms, int nchars)
{
    if (!s)
	s = &s_;
    if (*s->s_name == 'c')
    {
	natoms = fragile_class_count();
	if (natoms > 0 && testmess_makeroom(x, natoms * 2, 0))
	{
	    testmess_setnatoms(x, natoms);
	    fragile_class_getnames(x->x_message, natoms);
	}
    }
    else
    {
	if (natoms <= 0)
	    natoms = 100;
	if (nchars <= 0)
	    nchars = 10;
	if (testmess_makeroom(x, natoms * 2, 0))
	{
	    char buf[MAXPDSTRING], fmt[16];
	    int i = 0;
	    t_atom *ap;
	    testmess_setnatoms(x, natoms);
	    ap = x->x_message;
	    sprintf(fmt, "%%.%dx", nchars);
	    while (natoms--)
	    {
		snprintf(buf, MAXPDSTRING-1, fmt, i);
		SETSYMBOL(ap, gensym(buf));
		i++; ap++;
	    }
	}
    }
}

static void testmess_set(t_testmess *x, t_symbol *s, int ac, t_atom *av)
{
    t_symbol *csym = 0, *msym = 0;
    t_float f1 = 0., f2 = 0.;
    if (ac)
    {
	if (av->a_type == A_SYMBOL)
	    csym = av->a_w.w_symbol;
	else if (av->a_type == A_FLOAT)
	    f1 = av->a_w.w_float;
	if (ac > 1)
	{
	    if (av[1].a_type == A_SYMBOL)
		msym = av[1].a_w.w_symbol;
	    else if (av[1].a_type == A_FLOAT)
	    {
		if (csym)
		    f1 = av[1].a_w.w_float;
		else
		    f2 = av[1].a_w.w_float;
		if (ac > 2)
		{
		    if (av[2].a_type == A_SYMBOL)
			msym = av[2].a_w.w_symbol;
		    else if (csym && av[2].a_type == A_FLOAT)
		    f2 = av[2].a_w.w_float;
		}
	    }
	}
    }
    if (msym == gensym("stack"))
	x->x_method = msym, x->x_messfun = testmess_stackmess;
    else if (msym == gensym("heap"))
	x->x_method = msym, x->x_messfun = testmess_heapmess;
    else
    {
	x->x_method = gensym("prealloc");
	x->x_messfun = testmess_premess;
	x->x_tailwise = x->x_appendmode;
    }
    testmess_setnatoms(x, 0);
    if (csym)
	testmess_setnames(x, csym, (int)f1, (int)f2);
    else
	testmess_setnumbers(x, (int)f1, (int)f2);
}

static void testmess_free(t_testmess *x)
{
    if (x->x_messbuf != x->x_messini)
	freebytes(x->x_messbuf, x->x_size * sizeof(*x->x_messbuf));
}

static void *testmess_new(t_symbol *s, int ac, t_atom *av)
{
    t_testmess *x = (t_testmess *)pd_new(testmess_class);
    x->x_appendmode = 1;
    x->x_tailwise = 0;
    x->x_size = TESTMESS_INISIZE;
    x->x_messbuf = x->x_messini;
    outlet_new((t_object *)x, &s_anything);
    testmess_set(x, s, ac, av);
    return (x);
}

void testmess_setup(void)
{
    testmess_class = class_new(gensym("testmess"),
			       (t_newmethod)testmess_new,
			       (t_method)testmess_free,
			       sizeof(t_testmess), 0,
			       A_GIMME, 0);
    class_addbang(testmess_class, testmess_bang);
    class_addfloat(testmess_class, testmess_float);
    class_addsymbol(testmess_class, testmess_symbol);
    class_addanything(testmess_class, testmess_anything);
    class_addmethod(testmess_class, (t_method)testmess_set,
		    gensym("set"), A_GIMME, 0);
}
