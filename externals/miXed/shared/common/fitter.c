/* Copyright (c) 2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "m_pd.h"
#include "fitter.h"

#ifdef KRZYSZCZ
# include "loud.h"
# define FITTER_DEBUG
#else
# define loudbug_bug(msg)  fprintf(stderr, "BUG: %s\n", msg), bug(msg)
#endif

/* FIXME compatibility mode should be a standard Pd feature.  When it is,
   it will be possible to simplify the implementation.  Until then,
   we have to handle multiple copies of the 'fitterstate_mode' variable
   (coming from different externals), and the only way is multicasting
   through a symbol (#miXed). */
static t_symbol *fitterstate_mode = 0;

/* FIXME keep state in an extensible fitterstate_dictionary */
static t_symbol *fitterstate_test = 0;

typedef struct _fitterstate_client
{
    t_class                     *fc_owner;
    t_canvas                    *fc_canvas;
    t_fitterstate_callback       fc_callback;
    struct _fitterstate_client  *fc_next;
} t_fitterstate_client;

static t_fitterstate_client *fitterstate_clients = 0;
static t_class *fitterstate_class = 0;
static t_pd *fitterstate_target = 0;
static int fitterstate_ready = 0;
static t_symbol *fitterps_hashmiXed = 0;
static t_symbol *fitterps_mode = 0;
static t_symbol *fitterps_test = 0;
static t_symbol *fitterps_max = 0;
static t_symbol *fitterps_none = 0;

/* read access (query), called only from fitterstate_dosetup()
   or through "#miXed" */
static void fitterstate_bang(t_pd *x)
{
    if (fitterps_hashmiXed)
    {
	if (fitterstate_ready  /* do not reply to own request */
	    && fitterps_hashmiXed->s_thing)
	{
	    t_atom atout[2];
	    /* these proliferate for the third and subsequent
	       fitterstate_dosetup() calls... */
	    SETSYMBOL(&atout[0], fitterps_mode);
	    SETSYMBOL(&atout[1], fitterstate_mode);
	    typedmess(fitterps_hashmiXed->s_thing, gensym("reply"), 2, atout);
	    SETSYMBOL(&atout[0], fitterps_test);
	    SETSYMBOL(&atout[1], fitterstate_test);
	    typedmess(fitterps_hashmiXed->s_thing, gensym("reply"), 2, atout);
	}
    }
    else loudbug_bug("fitterstate_bang");
}

/* read access (query), called only through "#miXed" */
static void fitterstate_symbol(t_pd *x, t_symbol *s)
{
    if (fitterstate_ready && fitterps_hashmiXed && fitterps_hashmiXed->s_thing)
    {
	t_atom atout[2];
	if (s == fitterps_mode)
	{
	    SETSYMBOL(&atout[0], fitterps_mode);
	    SETSYMBOL(&atout[1], fitterstate_mode);
	    typedmess(fitterps_hashmiXed->s_thing, gensym("reply"), 2, atout);
	}
	else if (s == fitterps_test)
	{
	    SETSYMBOL(&atout[0], fitterps_test);
	    SETSYMBOL(&atout[1], fitterstate_test);
	    typedmess(fitterps_hashmiXed->s_thing, gensym("reply"), 2, atout);
	}
	else post("\"%s\": no such key in the miXed state",
		  (s ? s->s_name : "???"));
    }
    else loudbug_bug("fitterstate_symbol");
}

/* read access (reply), called only from fitter_dosetup() or through "#miXed" */
static void fitterstate_reply(t_pd *x, t_symbol *s1, t_symbol *s2)
{
    if (!s2 || s2 == &s_)
    {
	loudbug_bug("fitterstate_reply");
	s2 = fitterps_none;
    }
    if (s1 == fitterps_mode)
	fitterstate_mode = s2;
    else if (s1 == fitterps_test)
	fitterstate_test = s2;
}

/* write access, called only from fitter_setmode() or through "#miXed" */
static void fitterstate_set(t_pd *x, t_symbol *s1, t_symbol *s2)
{
    t_fitterstate_client *fc;
    if (s1 == fitterps_mode)
	fitterstate_mode = s2;
    else if (s1 == fitterps_test)
	fitterstate_test = s2;
    for (fc = fitterstate_clients; fc; fc = fc->fc_next)
	if (fc->fc_callback)
	    fc->fc_callback();
}

static void fitterstate_dosetup(int noquery)
{
    if (fitterstate_class || fitterstate_target)
	loudbug_bug("fitterstate_dosetup");
    fitterps_hashmiXed = gensym("#miXed");
    fitterps_mode = gensym("mode");
    fitterps_test = gensym("test");
    fitterps_max = gensym("max");
    fitterps_none = gensym("none");
    fitterstate_mode = fitterps_none;
    fitterstate_test = fitterps_none;
    fitterstate_class = class_new(fitterps_hashmiXed,
				  0, 0, sizeof(t_pd),
				  CLASS_PD | CLASS_NOINLET, 0);
    class_addbang(fitterstate_class, fitterstate_bang);
    class_addsymbol(fitterstate_class, fitterstate_symbol);
    class_addmethod(fitterstate_class,
		    (t_method)fitterstate_reply,
		    gensym("reply"), A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(fitterstate_class,
		    (t_method)fitterstate_set,
		    gensym("set"), A_SYMBOL, A_SYMBOL, 0);
    fitterstate_target = pd_new(fitterstate_class);
    pd_bind(fitterstate_target, fitterps_hashmiXed);
    if (!noquery)
	pd_bang(fitterps_hashmiXed->s_thing);
    fitterstate_ready = 1;
}

void fitter_setup(t_class *owner, t_fitterstate_callback callback)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    if (callback)
    {
	t_fitterstate_client *fc = getbytes(sizeof(*fc));
	fc->fc_owner = owner;
	fc->fc_canvas = 0;  /* a global client */
	fc->fc_callback = callback;
	fc->fc_next = fitterstate_clients;
	fitterstate_clients = fc;
    }
}

void fitter_drop(t_class *owner)
{
    if (fitterstate_class && fitterps_hashmiXed->s_thing)
    {
	t_fitterstate_client *fcp = 0,
	    *fc = fitterstate_clients;
	while (fc)
	{
	    if (fc->fc_owner == owner)
	    {
		if (fcp)
		    fcp->fc_next = fc->fc_next;
		else
		    fitterstate_clients = fc->fc_next;
		break;
	    }
	    fcp = fc;
	    fc = fc->fc_next;
	}
	if (fc)
	    freebytes(fc, sizeof(*fc));
	else
	    loudbug_bug("fitter_drop 1");
    }
    else loudbug_bug("fitter_drop 2");
}

t_float *fitter_getfloat(t_symbol *s)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    loudbug_bug("fitter_getfloat");
    return (0);
}

t_symbol *fitter_getsymbol(t_symbol *s)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    if (s == fitterps_mode)
	return (fitterstate_mode);
    else if (s == fitterps_test)
	return (fitterstate_test);
    else
    {
	loudbug_bug("fitter_getsymbol");
	return (0);
    }
}

void fitter_setmode(t_symbol *s)
{
    if (!s || s == &s_)
	s = fitterps_none;
    post("setting compatibility mode to '%s'", s->s_name);
    if (!fitterstate_class)
	fitterstate_dosetup(1);
    if (fitterps_hashmiXed->s_thing)
    {
	t_atom atout[2];
	SETSYMBOL(&atout[0], fitterps_mode);
	SETSYMBOL(&atout[1], s);
	typedmess(fitterps_hashmiXed->s_thing, gensym("set"), 2, atout);
    }
    else loudbug_bug("fitter_setmode");
}

t_symbol *fitter_getmode(void)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    return (fitterstate_mode);
}

void fittermax_set(void)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    fitter_setmode(fitterps_max);
}

int fittermax_get(void)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    return (fitterstate_mode == fitterps_max);
}

void fittermax_warning(t_class *c, char *fmt, ...)
{
    if (!fitterstate_class)
	fitterstate_dosetup(0);
    if (fitterstate_mode == fitterps_max)
    {
	char buf[MAXPDSTRING];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	post("'%s' class incompatibility warning:\n\t%s",
	     class_getname(c), buf);
	va_end(ap);
    }
}

void fittermax_rangewarning(t_class *c, int maxmax, char *what)
{
    fittermax_warning(c, "more than %d %s requested", maxmax, what);
}
