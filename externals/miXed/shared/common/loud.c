/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include "m_pd.h"
#include "loud.h"

#ifdef MSW
#define vsnprintf  _vsnprintf
#endif

/* The 'shared_' calls do not really belong here,
   LATER find them a permanent home. */

int shared_matchignorecase(char *test, char *pattern)
{
    char ct, cp;
    for (ct = *test, cp = *pattern; ct && cp; ct = *++test, cp = *++pattern)
	if (ct != cp
	    && ((ct < 'A' || ct > 'z')
		|| ((ct > 'Z' || ct + 32 != cp)
		    && (ct < 'a' || ct - 32 != cp))))
	    return (0);
    return (ct == cp);
}

struct _loudcontext
{
    t_pd      *lc_caller;    /* an object reporting trouble */
    char      *lc_callername;
    int        lc_cnsize;
    /* during object creation, use the following: */
    t_symbol  *lc_selector;  /* creation message selector (class name) */
    int        lc_ac;        /* creation message arguments */
    t_atom    *lc_av;        /* void out of creation context */
    int        lc_andindent;
};

#define LOUD_ERROR_DEFAULT  "error (miXed):"

char *loud_ordinal(int n)
{
    static char buf[16];  /* assuming 10-digit INT_MAX */
    sprintf(buf, "%dth", n);
    if (n < 0) n = -n;
    n %= 100;
    if (n > 20) n %= 10;
    if (n && n <= 3)
    {
	char *ptr = buf + strlen(buf) - 2;
	switch (n)
	{
	case 1: strcpy(ptr, "st"); break;
	case 2: strcpy(ptr, "nd"); break;
	case 3: strcpy(ptr, "rd"); break;
	}
    }
    return (buf);
}

void loud_error(t_pd *x, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    if (x)
    {
	startpost("%s's ", class_getname(*x));
	pd_error(x, buf);
    }
    else post("%s %s", LOUD_ERROR_DEFAULT, buf);
    va_end(ap);
}

void loud_errand(t_pd *x, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    post("%*s%s", (int)(x ? strlen(class_getname(*x)) + 10
			: strlen(LOUD_ERROR_DEFAULT) + 1), "", buf);
    va_end(ap);
}

void loud_syserror(t_pd *x, char *fmt, ...)
{
    if (fmt)
    {
	char buf[MAXPDSTRING];
	va_list ap;
	va_start(ap, fmt);
	vsprintf(buf, fmt, ap);
	loud_error(x, "%s (%s)", buf, strerror(errno));
	va_end(ap);
    }
    else loud_error(x, strerror(errno));
}

void loud_nomethod(t_pd *x, t_symbol *s)
{
    loud_error(x, "doesn't understand \"%s\"", s->s_name);
}

void loud_messarg(t_pd *x, t_symbol *s)
{
    loud_error(x, "bad arguments for message \"%s\"", s->s_name);
}

int loud_checkint(t_pd *x, t_float f, int *valuep, t_symbol *mess)
{
    if ((*valuep = (int)f) == f)
	return (1);
    else
    {
	static t_symbol *floatsym = 0;
	if (!floatsym)
	    floatsym = gensym("noninteger float");
	if (mess == &s_float)
	    loud_nomethod(x, floatsym);
	else if (mess)
	    loud_error(x, "\"%s\" argument invalid for message \"%s\"",
		       floatsym->s_name, mess->s_name);
	return (0);
    }
}

void loud_classarg(t_class *c)
{
    loud_error(0, "missing or bad arguments in \"%s\"", class_getname(c));
}

void loud_warning(t_pd *x, char *who, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    post("warning (%s): %s",
	 (x ? class_getname(*x) : (who ? who : "miXed")), buf);
    va_end(ap);
}

void loud_notimplemented(t_pd *x, char *name)
{
    if (name)
	loud_warning(x, 0, "\"%s\" method not implemented (yet)", name);
    else
	loud_warning(x, 0, "not implemented (yet)");
}

int loud_floatarg(t_class *c, int which, int ac, t_atom *av,
		  t_float *vp, t_float minval, t_float maxval,
		  int underaction, int overaction, char *what)
{
    int result = LOUD_ARGOK;
    if (which < ac)
    {
	av += which;
	if (av->a_type == A_FLOAT)
	{
	    t_float f = av->a_w.w_float;
	    if (f < minval)
	    {
		*vp = (underaction & LOUD_CLIP ? minval : f);
		if (underaction)
		    result = LOUD_ARGUNDER;
	    }
	    else if (f > maxval)
	    {
		*vp = (overaction & LOUD_CLIP ? maxval : f);
		if (overaction)
		    result = LOUD_ARGOVER;
	    }
	    else *vp = f;
	}
	else result = LOUD_ARGTYPE;
    }
    else result = LOUD_ARGMISSING;
    if (what)
    {
	switch (result)
	{
	case LOUD_ARGUNDER:
	    if (underaction & LOUD_WARN)
	    {
		if (underaction & LOUD_CLIP)
		    loud_warning(&c, 0, "%s rounded up to %g", what, minval);
		else
		    loud_warning(&c, 0, "less than %g %s requested",
				 minval, what);
	    }
	    break;
	case LOUD_ARGOVER:
	    if (overaction & LOUD_WARN)
	    {
		if (overaction & LOUD_CLIP)
		    loud_warning(&c, 0, "%s truncated to %g", what, maxval);
		else
		    loud_warning(&c, 0, "more than %g %s requested",
				 maxval, what);
	    }
	    break;
	case LOUD_ARGTYPE:
	    loud_error(0, "bad argument %d (%s)", which, class_getname(c));
	    break;
	default:;
	}
    }
    return (result);
}

void loudx_error(t_loudcontext *lc, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    if (lc->lc_caller)
    {
	startpost("%s's ", (lc->lc_callername ?
			    lc->lc_callername : class_getname(*lc->lc_caller)));
	pd_error(lc->lc_caller, buf);
    }
    else
    {
	if (lc->lc_callername)
	    post("error (%s): %s", lc->lc_callername, buf);
	else if (lc->lc_selector)
	    post("error (%s): %s", lc->lc_selector->s_name, buf);
	else
	    post("%s %s", LOUD_ERROR_DEFAULT, buf);
    }
    va_end(ap);
}

void loudx_errand(t_loudcontext *lc, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    post("%*s%s", lc->lc_andindent, "", buf);
    va_end(ap);
}

void loudx_nomethod(t_loudcontext *lc, t_symbol *s)
{
    loudx_error(lc, "doesn't understand \"%s\"", s->s_name);
}

void loudx_messarg(t_loudcontext *lc, t_symbol *s)
{
    loudx_error(lc, "bad arguments for message \"%s\"", s->s_name);
}

void loudx_warning(t_loudcontext *lc, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsprintf(buf, fmt, ap);
    if (lc->lc_callername)
	post("warning (%s): %s", lc->lc_callername, buf);
    else if (lc->lc_selector)
	post("warning (%s): %s", lc->lc_selector->s_name, buf);
    else
	post("warning (miXed): %s", buf);
    va_end(ap);
}

void loudx_setcontext(t_loudcontext *lc, t_pd *caller, char *callername,
		      t_symbol *s, int ac, t_atom *av)
{
    if (lc->lc_callername)
	freebytes(lc->lc_callername, lc->lc_cnsize);
    lc->lc_caller = caller;
    if (callername)
    {
	lc->lc_cnsize = strlen(callername) + 1;
	lc->lc_callername = getbytes(lc->lc_cnsize);
	strcpy(lc->lc_callername, callername);
    }
    else
    {
	lc->lc_callername = 0;
	lc->lc_cnsize = 0;
    }
    lc->lc_selector = s;
    lc->lc_ac = ac;
    lc->lc_av = av;
    if (callername)
	lc->lc_andindent = lc->lc_cnsize + 9;
    else if (caller)
	lc->lc_andindent = strlen(class_getname(*caller)) + 10;
    else if (s)
	lc->lc_andindent = strlen(s->s_name) + 10;
    else
	lc->lc_andindent = strlen(LOUD_ERROR_DEFAULT) + 1;
}

/* must call before going out of creation context */
void loudx_setcaller(t_loudcontext *lc, t_pd *caller, char *callerfmt, ...)
{
    va_list ap;
    va_start(ap, callerfmt);
    if (callerfmt)
    {
	char buf[MAXPDSTRING];
	vsprintf(buf, callerfmt, ap);
	loudx_setcontext(lc, caller, buf, lc->lc_selector, 0, 0);
    }
    else loudx_setcontext(lc, caller, 0, lc->lc_selector, 0, 0);
    va_end(ap);
}

t_symbol *loudx_getselector(t_loudcontext *lc)
{
    return (lc->lc_selector);
}

t_atom *loudx_getarguments(t_loudcontext *lc, int *acp)
{
    *acp = lc->lc_ac;
    return (lc->lc_av);
}

void loudx_freecontext(t_loudcontext *lc)
{
    if (lc->lc_callername)
	freebytes(lc->lc_callername, lc->lc_cnsize);
    freebytes(lc, sizeof(*lc));
}

t_loudcontext *loudx_newcontext(t_pd *caller, char *callername,
				t_symbol *s, int ac, t_atom *av)
{
    t_loudcontext *lc = getbytes(sizeof(*lc));
    lc->lc_callername = 0;
    loudx_setcontext(lc, caller, callername, s, ac, av);
    return (lc);
}

void loudbug_post(char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
    va_end(ap);
    fprintf(stderr, "%s\n", buf);
#ifdef MSW
    fflush(stderr);
#endif
}

void loudbug_startpost(char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
    va_end(ap);
    fputs(buf, stderr);
#ifdef MSW
    fflush(stderr);
#endif
}

void loudbug_stringpost(char *s)
{
    fputs(s, stderr);
#ifdef MSW
    fflush(stderr);
#endif
}

void loudbug_endpost(void)
{
    fputs("\n", stderr);
#ifdef MSW
    fflush(stderr);
#endif
}

void loudbug_postatom(int ac, t_atom *av)
{
    while (ac--)
    {
        char buf[MAXPDSTRING];
        atom_string(av++, buf, MAXPDSTRING);
	fprintf(stderr, " %s", buf);
#ifdef MSW
	fflush(stderr);
#endif
    }
}

void loudbug_postbinbuf(t_binbuf *bb)
{
    int ac = binbuf_getnatom(bb);
    t_atom *aprev = 0, *ap = binbuf_getvec(bb);
    while (ac--)
    {
        char buf[MAXPDSTRING];
        atom_string(ap, buf, MAXPDSTRING);
	if (aprev)
	{
	    if (aprev->a_type == A_SEMI)
		fprintf(stderr, "\n%s", buf);
	    else
		fprintf(stderr, " %s", buf);
	}
	else fprintf(stderr, "%s", buf);
#ifdef MSW
	fflush(stderr);
#endif
	aprev = ap++;
    }
    if (aprev)
    {
	fputs("\n", stderr);
#ifdef MSW
	fflush(stderr);
#endif
    }
}

void loudbug_bug(char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
    va_end(ap);
    fprintf(stderr, "miXed consistency check failed: %s\n", buf);
#ifdef MSW
    fflush(stderr);
#endif
    bug(buf);
}
