/* Copyright (c) 2002-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "hammer/file.h"

/* LATER profile for the bottlenecks of insertion and sorting */
/* LATER make sure that ``reentrancy protection hack'' is really working... */

#ifdef KRZYSZCZ
//#define COLL_DEBUG
#endif

enum { COLL_HEADRESET,
       COLL_HEADNEXT, COLL_HEADPREV,  /* distinction not used, currently */
       COLL_HEADDELETED };

typedef struct _collelem
{
    int                e_hasnumkey;
    int                e_numkey;
    t_symbol          *e_symkey;
    struct _collelem  *e_prev;
    struct _collelem  *e_next;
    int                e_size;
    t_atom            *e_data;
} t_collelem;

typedef struct _collcommon
{
    t_pd           c_pd;
    struct _coll  *c_refs;  /* used in read-banging and dirty flag handling */
    int            c_increation;
    int            c_volatile;
    int            c_selfmodified;
    int            c_entered;    /* a counter, LATER rethink */
    int            c_embedflag;  /* common field (CHECKED in 'TEXT' files) */
    t_symbol      *c_filename;   /* CHECKED common for all, read and write */
    t_canvas      *c_lastcanvas;
    t_hammerfile  *c_filehandle;
    t_collelem    *c_first;
    t_collelem    *c_last;
    t_collelem    *c_head;
    int            c_headstate;
} t_collcommon;

typedef struct _coll
{
    t_object       x_ob;
    t_canvas      *x_canvas;
    t_symbol      *x_name;
    t_collcommon  *x_common;
    t_hammerfile  *x_filehandle;
    t_outlet      *x_keyout;
    t_outlet      *x_filebangout;
    t_outlet      *x_dumpbangout;
    struct _coll  *x_next;
} t_coll;

static t_class *coll_class;
static t_class *collcommon_class;

static t_collelem *collelem_new(int ac, t_atom *av, int *np, t_symbol *s)
{
    t_collelem *ep = (t_collelem *)getbytes(sizeof(*ep));
    if (ep->e_hasnumkey = (np != 0))
	ep->e_numkey = *np;
    ep->e_symkey = s;
    ep->e_prev = ep->e_next = 0;
    if (ep->e_size = ac)
    {
	t_atom *ap = getbytes(ac * sizeof(*ap));
	ep->e_data = ap;
	if (av) while (ac--)
	    *ap++ = *av++;
	else while (ac--)
	{
	    SETFLOAT(ap, 0);
	    ap++;
	}
    }
    else ep->e_data = 0;
    return (ep);
}

static void collelem_free(t_collelem *ep)
{
    if (ep->e_data)
	freebytes(ep->e_data, ep->e_size * sizeof(*ep->e_data));
    freebytes(ep, sizeof(*ep));
}

/* CHECKME again... apparently c74 is not able to fix this for good */
/* result: 1 for ep1 < ep2, 0 for ep1 >= ep2, all symbols are < any float */
static int collelem_less(t_collelem *ep1, t_collelem *ep2, int ndx, int swap)
{
    int isless;
    if (swap)
    {
	t_collelem *ep = ep1;
	ep1 = ep2;
	ep2 = ep;
    }
    if (ndx < 0)
    {
	if (ep1->e_symkey)
	    isless =
		(ep2->e_symkey ? strcmp(ep1->e_symkey->s_name,
					ep2->e_symkey->s_name) < 0
		 : 1);  /* CHECKED incompatible with 4.07, but consistent */
	else if (ep2->e_symkey)
	    isless = 0;  /* CHECKED incompatible with 4.07, but consistent */
	else
	    isless = (ep1->e_numkey < ep2->e_numkey);  /* CHECKED in 4.07 */
    }
    else
    {
	t_atom *ap1 = (ndx < ep1->e_size ?
		       ep1->e_data + ndx : ep1->e_data + ep1->e_size - 1);
	t_atom *ap2 = (ndx < ep2->e_size ?
		       ep2->e_data + ndx : ep2->e_data + ep2->e_size - 1);
	if (ap1->a_type == A_FLOAT)
	{
 	    if (ap2->a_type == A_FLOAT)
		isless = (ap1->a_w.w_float < ap2->a_w.w_float);
 	    else if (ap2->a_type == A_SYMBOL)
		isless = 0;
	    else
		isless = 1;
	}
	else if (ap1->a_type == A_SYMBOL)
	{
	    if (ap2->a_type == A_FLOAT)
		isless = 1;
	    else if (ap2->a_type == A_SYMBOL)
		isless = (strcmp(ap1->a_w.w_symbol->s_name,
				 ap2->a_w.w_symbol->s_name) < 0);
	    else
		isless = 1;
	}
	else isless = 0;
    }
    return (isless);
}

static t_collelem *collcommon_numkey(t_collcommon *cc, int numkey)
{
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next)
	if (ep->e_hasnumkey && ep->e_numkey == numkey)
	    return (ep);
    return (0);
}

static t_collelem *collcommon_symkey(t_collcommon *cc, t_symbol *symkey)
{
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next)
	if (ep->e_symkey == symkey)
	    return (ep);
    return (0);
}

static void collcommon_takeout(t_collcommon *cc, t_collelem *ep)
{
    if (ep->e_prev)
	ep->e_prev->e_next = ep->e_next;
    else
	cc->c_first = ep->e_next;
    if (ep->e_next)
	ep->e_next->e_prev = ep->e_prev;
    else
	cc->c_last = ep->e_prev;
    if (cc->c_head == ep)
    {
	cc->c_head = ep->e_next;  /* asymmetric, LATER rethink */
	cc->c_headstate = COLL_HEADDELETED;
    }

}

static void collcommon_modified(t_collcommon *cc, int relinked)
{
    if (cc->c_increation)
	return;
    if (relinked)
    {
	cc->c_volatile = 1;
    }
    if (cc->c_embedflag)
    {
	t_coll *x;
	for (x = cc->c_refs; x; x = x->x_next)
	    if (x->x_canvas && glist_isvisible(x->x_canvas))
		canvas_dirty(x->x_canvas, 1);
    }
}

/* atomic collcommon modifiers:  clearall, remove, replace,
   putbefore, putafter, swaplinks, swapkeys, changesymkey, renumber, sort */

static void collcommon_clearall(t_collcommon *cc)
{
    if (cc->c_first)
    {
	t_collelem *ep1 = cc->c_first, *ep2;
	do
	{
	    ep2 = ep1->e_next;
	    collelem_free(ep1);
	}
	while (ep1 = ep2);
	cc->c_first = cc->c_last = 0;
	cc->c_head = 0;
	cc->c_headstate = COLL_HEADRESET;
	collcommon_modified(cc, 1);
    }
}

static void collcommon_remove(t_collcommon *cc, t_collelem *ep)
{
    collcommon_takeout(cc, ep);
    collelem_free(ep);
    collcommon_modified(cc, 1);
}

static void collcommon_replace(t_collcommon *cc, t_collelem *ep,
			       int ac, t_atom *av, int *np, t_symbol *s)
{
    if (ep->e_hasnumkey = (np != 0))
	ep->e_numkey = *np;
    ep->e_symkey = s;
    if (ac)
    {
	int i = ac;
	t_atom *ap;
	if (ep->e_data)
	{
	    if (ep->e_size != ac)
		ap = resizebytes(ep->e_data,
				 ep->e_size * sizeof(*ap), ac * sizeof(*ap));
	    else ap = ep->e_data;
	}
	else
	    ap = getbytes(ac * sizeof(*ap));
	ep->e_data = ap;
	if (av) while (i --)
	    *ap++ = *av++;
	else while (i --)
	{
	    SETFLOAT(ap, 0);
	    ap++;
	}
    }
    else
    {
	if (ep->e_data)
	    freebytes(ep->e_data, ep->e_size * sizeof(*ep->e_data));
	ep->e_data = 0;
    }
    ep->e_size = ac;
    collcommon_modified(cc, 0);
}

static void collcommon_putbefore(t_collcommon *cc,
				 t_collelem *ep, t_collelem *next)
{
    if (next)
    {
	ep->e_next = next;
	if (ep->e_prev = next->e_prev)
	    ep->e_prev->e_next = ep;
	else
	    cc->c_first = ep;
	next->e_prev = ep;
    }
    else if (cc->c_first || cc->c_last)
	loudbug_bug("collcommon_putbefore");
    else
	cc->c_first = cc->c_last = ep;
    collcommon_modified(cc, 1);
}

static void collcommon_putafter(t_collcommon *cc,
				t_collelem *ep, t_collelem *prev)
{
    if (prev)
    {
	ep->e_prev = prev;
	if (ep->e_next = prev->e_next)
	    ep->e_next->e_prev = ep;
	else
	    cc->c_last = ep;
	prev->e_next = ep;
    }
    else if (cc->c_first || cc->c_last)
	loudbug_bug("collcommon_putafter");
    else
	cc->c_first = cc->c_last = ep;
    collcommon_modified(cc, 1);
}

/* LATER consider making it faster, if there is a real need.
   Now called only in the sort routine, once per sort. */
static void collcommon_swaplinks(t_collcommon *cc,
				 t_collelem *ep1, t_collelem *ep2)
{
    if (ep1 != ep2)
    {
	t_collelem *prev1 = ep1->e_prev, *prev2 = ep2->e_prev;
	if (prev1 == ep2)
	{
	    collcommon_takeout(cc, ep2);
	    collcommon_putafter(cc, ep2, ep1);
	}
	else if (prev2 == ep1)
	{
	    collcommon_takeout(cc, ep1);
	    collcommon_putafter(cc, ep1, ep2);
	}
	else if (prev1)
	{
	    if (prev2)
	    {
		collcommon_takeout(cc, ep1);
		collcommon_takeout(cc, ep2);
		collcommon_putafter(cc, ep1, prev2);
		collcommon_putafter(cc, ep2, prev1);
	    }
	    else
	    {
		t_collelem *next2 = ep2->e_next;
		collcommon_takeout(cc, ep1);
		collcommon_takeout(cc, ep2);
		collcommon_putbefore(cc, ep1, next2);
		collcommon_putafter(cc, ep2, prev1);
	    }
	}
	else if (prev2)
	{
	    t_collelem *next1 = ep1->e_next;
	    collcommon_takeout(cc, ep1);
	    collcommon_takeout(cc, ep2);
	    collcommon_putafter(cc, ep1, prev2);
	    collcommon_putbefore(cc, ep2, next1);
	}
	else loudbug_bug("collcommon_swaplinks");
    }
}

static void collcommon_swapkeys(t_collcommon *cc,
				t_collelem *ep1, t_collelem *ep2)
{
    int hasnumkey = ep2->e_hasnumkey, numkey = ep2->e_numkey;
    t_symbol *symkey = ep2->e_symkey;
    ep2->e_hasnumkey = ep1->e_hasnumkey;
    ep2->e_numkey = ep1->e_numkey;
    ep2->e_symkey = ep1->e_symkey;
    ep1->e_hasnumkey = hasnumkey;
    ep1->e_numkey = numkey;
    ep1->e_symkey = symkey;
    collcommon_modified(cc, 0);
}

static void collcommon_changesymkey(t_collcommon *cc,
				    t_collelem *ep, t_symbol *s)
{
    ep->e_symkey = s;
    collcommon_modified(cc, 0);
}

static void collcommon_renumber(t_collcommon *cc, int startkey)
{
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next)
	if (ep->e_hasnumkey)
	    ep->e_numkey = startkey++;
    collcommon_modified(cc, 0);
}

/* LATER choose a better algo, after coll's storage structures stabilize.
   Note, that even the simple insertion sort below (n-square) might prove better
   for bi-directional lists, than theoretically efficient algo (nlogn) requiring
   random access emulation.  Avoiding recursion is not a bad idea, too. */
static void collcommon_sort(t_collcommon *cc, int descending, int ndx)
{
    t_collelem *min = cc->c_first;
    t_collelem *ep;
    if (min && (ep = min->e_next))
    {
	cc->c_increation = 1;
	/* search for a sentinel element */
 	do
	    if (collelem_less(ep, min, ndx, descending))
		min = ep;
	while (ep = ep->e_next);
	/* prepend it */
	collcommon_swaplinks(cc, cc->c_first, min);
	/* sort */
 	ep = min->e_next->e_next;
	while (ep)
	{
	    t_collelem *next = ep->e_next;
	    for (min = ep->e_prev;
		 min &&  /* LATER remove */
		     collelem_less(ep, min, ndx, descending);
		 min = min->e_prev);
	    if (!min)  /* LATER remove */
		loudbug_bug("collcommon_sort");
	    else if (ep != min->e_next)
	    {
		collcommon_takeout(cc, ep);
		collcommon_putafter(cc, ep, min);
	    }
	    ep = next;
	}
	cc->c_increation = 0;
	collcommon_modified(cc, 1);
    }
}

static void collcommon_adddata(t_collcommon *cc, t_collelem *ep,
			       int ac, t_atom *av)
{
    if (ac)
    {
	t_atom *ap;
	int newsize = ep->e_size + ac;
	if (ep->e_data)
	    ap = resizebytes(ep->e_data,
			     ep->e_size * sizeof(*ap), newsize * sizeof(*ap));
	else
	{
	    ep->e_size = 0;  /* redundant, hopefully */
	    ap = getbytes(newsize * sizeof(*ap));
	}
	ep->e_data = ap;
	ap += ep->e_size;
	if (av) while (ac--)
	    *ap++ = *av++;
	else while (ac--)
	{
	    SETFLOAT(ap, 0);
	    ap++;
	}
	ep->e_size = newsize;
	collcommon_modified(cc, 0);
    }
}

static t_collelem *collcommon_tonumkey(t_collcommon *cc, int numkey,
				       int ac, t_atom *av, int replace)
{
    t_collelem *old = collcommon_numkey(cc, numkey), *new;
    if (old && replace)
	collcommon_replace(cc, new = old, ac, av, &numkey, 0);
    else
    {
	new = collelem_new(ac, av, &numkey, 0);
	if (old)
	{
	    collcommon_putbefore(cc, new, old);
	    do
		if (old->e_hasnumkey)
		    /* CHECKED incremented up to the last one; incompatible:
		       elements with numkey == 0 not incremented (a bug?) */
		    old->e_numkey++;
	    while (old = old->e_next);
	}
	else
	{
	    /* CHECKED negative numkey always put before the last element,
	       zero numkey always becomes the new head */
	    int closestkey = 0;
	    t_collelem *closest = 0, *ep;
	    for (ep = cc->c_first; ep; ep = ep->e_next)
	    {
		if (ep->e_hasnumkey)
		{
		    if (numkey >= closestkey && numkey <= ep->e_numkey)
		    {
			collcommon_putbefore(cc, new, ep);
			break;
		    }
		    closestkey = ep->e_numkey;
		}
		closest = ep;
	    }
	    if (!ep)
	    {
		if (numkey <= closestkey)
		    collcommon_putbefore(cc, new, closest);
		else
		    collcommon_putafter(cc, new, closest);
	    }
	}
    }
    return (new);
}

static t_collelem *collcommon_tosymkey(t_collcommon *cc, t_symbol *symkey,
				       int ac, t_atom *av, int replace)
{
    t_collelem *old = collcommon_symkey(cc, symkey), *new;
    if (old && replace)
	collcommon_replace(cc, new = old, ac, av, 0, symkey);
    else
	collcommon_putafter(cc, new = collelem_new(ac, av, 0, symkey),
			    cc->c_last);
    return (new);
}

static int collcommon_fromatoms(t_collcommon *cc, int ac, t_atom *av)
{
    int hasnumkey = 0, numkey;
    t_symbol *symkey = 0;
    int size = 0;
    t_atom *data = 0;
    int nlines = 0;
    cc->c_increation = 1;
    collcommon_clearall(cc);
    while (ac--)
    {
	if (data)
	{
	    if (av->a_type == A_SEMI)
	    {
		t_collelem *ep = collelem_new(size, data,
					      hasnumkey ? &numkey : 0, symkey);
		collcommon_putafter(cc, ep, cc->c_last);
		hasnumkey = 0;
		symkey = 0;
		data = 0;
		nlines++;
	    }
	    if (av->a_type == A_COMMA)
	    {
		/* CHECKED rejecting a comma */
		collcommon_clearall(cc);  /* LATER rethink */
		cc->c_increation = 0;
		return (-nlines);
	    }
	    else size++;
	}
	else if (av->a_type == A_COMMA)
	{
	    size = 0;
	    data = av + 1;
	}
	else if (av->a_type == A_SYMBOL)
	    symkey = av->a_w.w_symbol;
	else if (av->a_type == A_FLOAT &&
		 loud_checkint(0, av->a_w.w_float, &numkey, 0))
	    hasnumkey = 1;
	else
	{
	    loud_error(0, "coll: bad atom");
	    collcommon_clearall(cc);  /* LATER rethink */
	    cc->c_increation = 0;
	    return (-nlines);
	}
	av++;
    }
    if (data)
    {
	loud_error(0, "coll: incomplete");
	collcommon_clearall(cc);  /* LATER rethink */
	cc->c_increation = 0;
	return (-nlines);
    }
    cc->c_increation = 0;
    return (nlines);
}

static int collcommon_frombinbuf(t_collcommon *cc, t_binbuf *bb)
{
    return (collcommon_fromatoms(cc, binbuf_getnatom(bb), binbuf_getvec(bb)));
}

static void collcommon_doread(t_collcommon *cc, t_symbol *fn, t_canvas *cv)
{
    t_binbuf *bb;
    char buf[MAXPDSTRING];
    if (!fn && !(fn = cc->c_filename))  /* !fn: 'readagain' */
	return;
    /* FIXME use open_via_path() */
    if (cv || (cv = cc->c_lastcanvas))  /* !cv: 'read' w/o arg, 'readagain' */
	canvas_makefilename(cv, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    if (!cc->c_refs)
    {
	/* loading during object creation --
	   avoid binbuf_read()'s complaints, LATER rethink */
	FILE *fp;
	if (!(fp = sys_fopen(buf, "r")))
	{
	    loud_warning(&coll_class, 0, "no coll file '%s'", buf);
	    return;
	}
	fclose(fp);
    }
    bb = binbuf_new();
    if (binbuf_read(bb, buf, "", 0))
	loud_error(0, "coll: error reading text file '%s'", fn->s_name);
    else
    {
	int nlines = collcommon_frombinbuf(cc, bb);
	if (nlines > 0)
	{
	    t_coll *x;
	    /* LATER consider making this more robust */
	    for (x = cc->c_refs; x; x = x->x_next)
		outlet_bang(x->x_filebangout);
	    cc->c_lastcanvas = cv;
	    cc->c_filename = fn;
	    post("coll: finished reading %d lines from text file '%s'",
		 nlines, fn->s_name);
	}
	else if (nlines < 0)
	    loud_error(0, "coll: error in line %d of text file '%s'",
		       1 - nlines, fn->s_name);
	else
	    loud_error(0, "coll: error reading text file '%s'", fn->s_name);
	if (cc->c_refs)
	    collcommon_modified(cc, 1);
    }
    binbuf_free(bb);
}

static void collcommon_readhook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    collcommon_doread((t_collcommon *)z, fn, 0);
}

static void collcommon_tobinbuf(t_collcommon *cc, t_binbuf *bb)
{
    t_collelem *ep;
    t_atom at[3];
    for (ep = cc->c_first; ep; ep = ep->e_next)
    {
	t_atom *ap = at;
	int cnt = 1;
	if (ep->e_hasnumkey)
	{
	    SETFLOAT(ap, ep->e_numkey);
	    ap++; cnt++;
	}
	if (ep->e_symkey)
	{
	    SETSYMBOL(ap, ep->e_symkey);
	    ap++; cnt++;
	}
	SETCOMMA(ap);
	binbuf_add(bb, cnt, at);
	binbuf_add(bb, ep->e_size, ep->e_data);
	binbuf_addsemi(bb);
    }
}

static void collcommon_dowrite(t_collcommon *cc, t_symbol *fn, t_canvas *cv)
{
    t_binbuf *bb;
    int ac;
    t_atom *av;
    char buf[MAXPDSTRING];
    if (!fn && !(fn = cc->c_filename))  /* !fn: 'writeagain' */
	return;
    if (cv || (cv = cc->c_lastcanvas))  /* !cv: 'write' w/o arg, 'writeagain' */
	canvas_makefilename(cv, fn->s_name, buf, MAXPDSTRING);
    else
    {
    	strncpy(buf, fn->s_name, MAXPDSTRING);
    	buf[MAXPDSTRING-1] = 0;
    }
    bb = binbuf_new();
    collcommon_tobinbuf(cc, bb);
    if (binbuf_write(bb, buf, "", 0))
	loud_error(0, "coll: error writing text file '%s'", fn->s_name);
    else
    {
	cc->c_lastcanvas = cv;
	cc->c_filename = fn;
    }
    binbuf_free(bb);
}

static void collcommon_writehook(t_pd *z, t_symbol *fn, int ac, t_atom *av)
{
    collcommon_dowrite((t_collcommon *)z, fn, 0);
}

static void coll_embedhook(t_pd *z, t_binbuf *bb, t_symbol *bindsym)
{
    t_coll *x = (t_coll *)z;
    t_collcommon *cc = x->x_common;
    if (cc->c_embedflag)
    {
	t_collelem *ep;
	t_atom at[6];
	binbuf_addv(bb, "ssii;", bindsym, gensym("flags"), 1, 0);
	SETSYMBOL(at, bindsym);
	for (ep = cc->c_first; ep; ep = ep->e_next)
	{
	    t_atom *ap = at + 1;
	    int cnt;
	    if (ep->e_hasnumkey && ep->e_symkey)
	    {
		SETSYMBOL(ap, gensym("nstore"));
		ap++;
		SETSYMBOL(ap, ep->e_symkey);
		ap++;
		SETFLOAT(ap, ep->e_numkey);
		cnt = 4;
	    }
	    else if (ep->e_symkey)
	    {
		SETSYMBOL(ap, gensym("store"));
		ap++;
		SETSYMBOL(ap, ep->e_symkey);
		cnt = 3;
	    }
	    else
	    {
		SETFLOAT(ap, ep->e_numkey);
		cnt = 2;
	    }
	    binbuf_add(bb, cnt, at);
	    binbuf_add(bb, ep->e_size, ep->e_data);
	    binbuf_addsemi(bb);
	}
    }
}

static void collcommon_editorhook(t_pd *z, t_symbol *s, int ac, t_atom *av)
{
    int nlines = collcommon_fromatoms((t_collcommon *)z, ac, av);
    if (nlines < 0)
	loud_error(0, "coll: editing error in line %d", 1 - nlines);
}

static void collcommon_free(t_collcommon *cc)
{
    t_collelem *ep1, *ep2 = cc->c_first;
    while (ep1 = ep2)
    {
	ep2 = ep1->e_next;
	collelem_free(ep1);
    }
}

static void *collcommon_new(void)
{
    t_collcommon *cc = (t_collcommon *)pd_new(collcommon_class);
    cc->c_embedflag = 0;
    cc->c_first = cc->c_last = 0;
    cc->c_head = 0;
    cc->c_headstate = COLL_HEADRESET;
    return (cc);
}

static t_collcommon *coll_checkcommon(t_coll *x)
{
    if (x->x_name &&
	x->x_common != (t_collcommon *)pd_findbyclass(x->x_name,
						      collcommon_class))
    {
	loudbug_bug("coll_checkcommon");
	return (0);
    }
    return (x->x_common);
}

static void coll_unbind(t_coll *x)
{
    /* LATER consider calling coll_checkcommon(x) */
    t_collcommon *cc = x->x_common;
    t_coll *prev, *next;
    if ((prev = cc->c_refs) == x)
    {
	if (!(cc->c_refs = x->x_next))
	{
	    hammerfile_free(cc->c_filehandle);
	    collcommon_free(cc);
	    if (x->x_name) pd_unbind(&cc->c_pd, x->x_name);
	    pd_free(&cc->c_pd);
	}
    }
    else if (prev)
    {
	while (next = prev->x_next)
	{
	    if (next == x)
	    {
		prev->x_next = next->x_next;
		break;
	    }
	    prev = next;
	}
    }
    x->x_common = 0;
    x->x_name = 0;
    x->x_next = 0;
}

static void coll_bind(t_coll *x, t_symbol *name)
{
    t_collcommon *cc = 0;
    if (name == &s_)
	name = 0;
    else if (name)
	cc = (t_collcommon *)pd_findbyclass(name, collcommon_class);
    if (!cc)
    {
	cc = (t_collcommon *)collcommon_new();
	cc->c_refs = 0;
	cc->c_increation = 0;
	if (name)
	{
	    pd_bind(&cc->c_pd, name);
	    /* LATER rethink canvas unpredictability */
	    collcommon_doread(cc, name, x->x_canvas);
	}
	else
	{
	    cc->c_filename = 0;
	    cc->c_lastcanvas = 0;
	}
	cc->c_filehandle = hammerfile_new((t_pd *)cc, 0, collcommon_readhook,
					  collcommon_writehook,
					  collcommon_editorhook);
    }
    x->x_common = cc;
    x->x_name = name;
    x->x_next = cc->c_refs;
    cc->c_refs = x;
}

static int coll_rebind(t_coll *x, t_symbol *name)
{
    t_collcommon *cc;
    if (name && name != &s_ &&
	(cc = (t_collcommon *)pd_findbyclass(name, collcommon_class)))
    {
	coll_unbind(x);
	x->x_common = cc;
	x->x_name = name;
	x->x_next = cc->c_refs;
	cc->c_refs = x;
	return (1);
    }
    else return (0);
}

static void coll_dooutput(t_coll *x, int ac, t_atom *av)
{
    if (ac > 1)
    {
	if (av->a_type == A_FLOAT)
	    outlet_list(((t_object *)x)->ob_outlet, &s_list, ac, av);
	else if (av->a_type == A_SYMBOL)
	    outlet_anything(((t_object *)x)->ob_outlet,
			    av->a_w.w_symbol, ac-1, av+1);
    }
    else if (ac)
    {
	if (av->a_type == A_FLOAT)
	    outlet_float(((t_object *)x)->ob_outlet, av->a_w.w_float);
	else if (av->a_type == A_SYMBOL)
	    outlet_symbol(((t_object *)x)->ob_outlet, av->a_w.w_symbol);
    }
}

static void coll_keyoutput(t_coll *x, t_collelem *ep)
{
    t_collcommon *cc = x->x_common;
    if (!cc->c_entered++) cc->c_selfmodified = 0;
    cc->c_volatile = 0;
    if (ep->e_hasnumkey)
	outlet_float(x->x_keyout, ep->e_numkey);
    else if (ep->e_symkey)
	outlet_symbol(x->x_keyout, ep->e_symkey);
    else
	outlet_float(x->x_keyout, 0);
    if (cc->c_volatile) cc->c_selfmodified = 1;
    cc->c_entered--;
}

static t_collelem *coll_findkey(t_coll *x, t_atom *key, t_symbol *mess)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep = 0;
    if (key->a_type == A_FLOAT)
    {
	int numkey;
	if (loud_checkint((t_pd *)x, key->a_w.w_float, &numkey, mess))
	    ep = collcommon_numkey(cc, numkey);
	else
	    mess = 0;
    }
    else if (key->a_type == A_SYMBOL)
	ep = collcommon_symkey(cc, key->a_w.w_symbol);
    else if (mess)
    {
	loud_messarg((t_pd *)x, mess);
	mess = 0;
    }
    if (!ep && mess)
	loud_error((t_pd *)x, "no such key");
    return (ep);
}

static t_collelem *coll_tokey(t_coll *x, t_atom *key, int ac, t_atom *av,
			      int replace, t_symbol *mess)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep = 0;
    if (key->a_type == A_FLOAT)
    {
	int numkey;
	if (loud_checkint((t_pd *)x, key->a_w.w_float, &numkey, mess))
	    ep = collcommon_tonumkey(cc, numkey, ac, av, replace);
    }
    else if (key->a_type == A_SYMBOL)
	ep = collcommon_tosymkey(cc, key->a_w.w_symbol, ac, av, replace);
    else if (mess)
	loud_messarg((t_pd *)x, mess);
    return (ep);
}

static t_collelem *coll_firsttyped(t_coll *x, int ndx, t_atomtype type)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next)
	if (ep->e_size > ndx && ep->e_data[ndx].a_type == type)
	    return (ep);
    return (0);
}

/* the methods */

static void coll_float(t_coll *x, t_float f)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep;
    int numkey;
    if (loud_checkint((t_pd *)x, f, &numkey, &s_float) &&
	(ep = collcommon_numkey(cc, numkey)))
    {
	coll_keyoutput(x, ep);
	if (!cc->c_selfmodified || (ep = collcommon_numkey(cc, numkey)))
	    coll_dooutput(x, ep->e_size, ep->e_data);
    }
}

static void coll_symbol(t_coll *x, t_symbol *s)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep;
    if (ep = collcommon_symkey(cc, s))
    {
	coll_keyoutput(x, ep);
	if (!cc->c_selfmodified || (ep = collcommon_symkey(cc, s)))
	    coll_dooutput(x, ep->e_size, ep->e_data);
    }
}

static void coll_list(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av->a_type == A_FLOAT)
	coll_tokey(x, av, ac-1, av+1, 1, &s_list);
    else
	loud_messarg((t_pd *)x, &s_list);
}

static void coll_anything(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    coll_symbol(x, s);
}

static void coll_store(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2)
	coll_tokey(x, av, ac-1, av+1, 1, s);
    else
	loud_messarg((t_pd *)x, s);
}

static void coll_nstore(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 3)
    {
	t_collcommon *cc = x->x_common;
	t_collelem *ep;
	int numkey;
	if (av->a_type == A_FLOAT && av[1].a_type == A_SYMBOL)
	{
	    if (loud_checkint((t_pd *)x, av->a_w.w_float, &numkey, s))
	    {
		if (ep = collcommon_symkey(cc, av[1].a_w.w_symbol))
		    collcommon_remove(cc, ep);
		ep = collcommon_tonumkey(cc, numkey, ac-2, av+2, 1);
		ep->e_symkey = av[1].a_w.w_symbol;
	    }
	}
	else if (av->a_type == A_SYMBOL && av[1].a_type == A_FLOAT)
	{
	    if (loud_checkint((t_pd *)x, av[1].a_w.w_float, &numkey, s))
	    {
		if (ep = collcommon_numkey(cc, numkey))
		    collcommon_remove(cc, ep);
		ep = collcommon_tosymkey(cc, av->a_w.w_symbol, ac-2, av+2, 1);
		ep->e_hasnumkey = 1;
		ep->e_numkey = numkey;
	    }
	}
	else loud_messarg((t_pd *)x, s);
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_insert(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av->a_type == A_FLOAT)
	coll_tokey(x, av, ac-1, av+1, 0, s);
    else
	loud_messarg((t_pd *)x, s);
}

static void coll_remove(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_collelem *ep;
	if (ep = coll_findkey(x, av, s))
	    collcommon_remove(x->x_common, ep);
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_delete(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_collelem *ep;
	if (ep = coll_findkey(x, av, s))
	{
	    if (av->a_type == A_FLOAT)
	    {
		int numkey = ep->e_numkey;
		t_collelem *next;
		for (next = ep->e_next; next; next = next->e_next)
		    if (next->e_hasnumkey && next->e_numkey > numkey)
			next->e_numkey--;
	    }
	    collcommon_remove(x->x_common, ep);
	}
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_assoc(t_coll *x, t_symbol *s, t_floatarg f)
{
    int numkey;
    if (loud_checkint((t_pd *)x, f, &numkey, gensym("assoc")))
    {
	t_collcommon *cc = x->x_common;
	t_collelem *ep1, *ep2;
	if ((ep1 = collcommon_numkey(cc, numkey)) &&
	    ep1->e_symkey != s)  /* LATER rethink */
	{
	    if (ep2 = collcommon_symkey(cc, s))
		collcommon_remove(cc, ep2);
	    collcommon_changesymkey(cc, ep1, s);
	}
    }
}
		
static void coll_deassoc(t_coll *x, t_symbol *s, t_floatarg f)
{
    int numkey;
    if (loud_checkint((t_pd *)x, f, &numkey, gensym("deassoc")))
    {
	t_collcommon *cc = x->x_common;
	t_collelem *ep;
	if (ep = collcommon_numkey(cc, numkey))
	    collcommon_changesymkey(cc, ep, 0);
    }
}

static void coll_subsym(t_coll *x, t_symbol *s1, t_symbol *s2)
{
    t_collelem *ep;
    if (s1 != s2 && (ep = collcommon_symkey(x->x_common, s2)))
	collcommon_changesymkey(x->x_common, ep, s1);
}

static void coll_renumber(t_coll *x, t_floatarg f)
{
    int startkey;
    if (loud_checkint((t_pd *)x, f, &startkey, gensym("renumber")))
	collcommon_renumber(x->x_common, startkey);
}

static void coll_merge(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2)
    {
	t_collcommon *cc = x->x_common;
	t_collelem *ep;
	if (av->a_type == A_FLOAT)
	{
	    int numkey;
	    if (loud_checkint((t_pd *)x, av->a_w.w_float, &numkey, s))
	    {
		if (ep = collcommon_numkey(cc, numkey))
		    collcommon_adddata(cc, ep, ac-1, av+1);
		else  /* LATER consider defining collcommon_toclosest() */
		    collcommon_tonumkey(cc, numkey, ac-1, av+1, 1);
	    }
	}
	else if (av->a_type == A_SYMBOL)
	{
	    if (ep = collcommon_symkey(cc, av->a_w.w_symbol))
		collcommon_adddata(cc, ep, ac-1, av+1);
	    else
	    {
		ep = collelem_new(ac-1, av+1, 0, av->a_w.w_symbol);
		collcommon_putafter(cc, ep, cc->c_last);
	    }
	}
	else loud_messarg((t_pd *)x, s);
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_sub(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_collelem *ep;
	if (ep = coll_findkey(x, av, s))
	{
	    t_collcommon *cc = x->x_common;
	    t_atom *key = av++;
	    ac--;
	    while (ac >= 2)
	    {
		if (av->a_type == A_FLOAT)
		{
		    int ndx;
		    if (loud_checkint((t_pd *)x, av->a_w.w_float, &ndx, 0)
			&& ndx >= 1 && ndx <= ep->e_size)
			ep->e_data[ndx-1] = av[1];
		}
		ac -= 2;
		av += 2;
	    }
	    if (s == gensym("sub"))
	    {
		coll_keyoutput(x, ep);
		if (!cc->c_selfmodified || (ep = coll_findkey(x, key, 0)))
		    coll_dooutput(x, ep->e_size, ep->e_data);
	    }
	}
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_sort(t_coll *x, t_floatarg f1, t_floatarg f2)
{
    int dir, ndx;
    if (loud_checkint((t_pd *)x, f1, &dir, gensym("sort")) &&
	loud_checkint((t_pd *)x, f2, &ndx, gensym("sort")))
	collcommon_sort(x->x_common, (dir < 0 ? 0 : 1),
			(ndx < 0 ? -1 : (ndx ? ndx - 1 : 0)));
}

static void coll_clear(t_coll *x)
{
    collcommon_clearall(x->x_common);
}

/* According to the refman, the data should be swapped, rather than the keys
   -- easy here, but apparently c74 people have chosen to avoid some effort
   needed in case of their implementation... */
static void coll_swap(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac == 2)
    {
	t_collelem *ep1, *ep2;
	if ((ep1 = coll_findkey(x, av, s)) &&
	    (ep2 = coll_findkey(x, av + 1, s)))
	    collcommon_swapkeys(x->x_common, ep1, ep2);
    }
    else loud_messarg((t_pd *)x, s);
}

/* CHECKED traversal direction change is consistent with the general rule:
   'next' always outputs e_next of a previous output, and 'prev' always
   outputs e_prev, whether preceded by 'prev', or by 'next'.  This is
   currently implemented by pre-updating of the head (which is inhibited
   if there was no previous output, i.e. after 'goto', 'end', or collection
   initialization).  CHECKME again. */

static void coll_next(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    if (cc->c_headstate != COLL_HEADRESET &&
	cc->c_headstate != COLL_HEADDELETED)  /* asymmetric, LATER rethink */
    {
	if (cc->c_head)
	    cc->c_head = cc->c_head->e_next;
	if (!cc->c_head && !(cc->c_head = cc->c_first))  /* CHECKED wrapping */
	    return;
    }
    else if (!cc->c_head && !(cc->c_head = cc->c_first))
	return;
    cc->c_headstate = COLL_HEADNEXT;
    coll_keyoutput(x, cc->c_head);
    if (cc->c_head)
	coll_dooutput(x, cc->c_head->e_size, cc->c_head->e_data);
    else if (!cc->c_selfmodified)
	loudbug_bug("coll_next");  /* LATER rethink */
}

static void coll_prev(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    if (cc->c_headstate != COLL_HEADRESET)
    {
	if (cc->c_head)
	    cc->c_head = cc->c_head->e_prev;
	if (!cc->c_head && !(cc->c_head = cc->c_last))  /* CHECKED wrapping */
	    return;
    }
    else if (!cc->c_head && !(cc->c_head = cc->c_first))
	return;
    cc->c_headstate = COLL_HEADPREV;
    coll_keyoutput(x, cc->c_head);
    if (cc->c_head)
	coll_dooutput(x, cc->c_head->e_size, cc->c_head->e_data);
    else if (!cc->c_selfmodified)
	loudbug_bug("coll_prev");  /* LATER rethink */
}

static void coll_end(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    cc->c_head = cc->c_last;
    cc->c_headstate = COLL_HEADRESET;
}

static void coll_goto(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac)
    {
	t_collelem *ep = coll_findkey(x, av, s);
	if (ep)
	{
	    t_collcommon *cc = x->x_common;
	    cc->c_head = ep;
	    cc->c_headstate = COLL_HEADRESET;
	}
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_nth(t_coll *x, t_symbol *s, int ac, t_atom *av)
{
    if (ac >= 2 && av[1].a_type == A_FLOAT)
    {
	int ndx;
	t_collelem *ep;
	if (loud_checkint((t_pd *)x, av[1].a_w.w_float, &ndx, s) &&
	    (ep = coll_findkey(x, av, s)) &&
	    ep->e_size >= ndx)
	{
	    t_atom *ap = ep->e_data + --ndx;
	    if (ap->a_type == A_FLOAT)
		outlet_float(((t_object *)x)->ob_outlet, ap->a_w.w_float);
	    else if (ap->a_type == A_SYMBOL)
		outlet_symbol(((t_object *)x)->ob_outlet, ap->a_w.w_symbol);
	}
    }
    else loud_messarg((t_pd *)x, s);
}

static void coll_length(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep = cc->c_first;
    int result = 0;
    while (ep) result++, ep = ep->e_next;
    outlet_float(((t_object *)x)->ob_outlet, result);
}

static void coll_min(t_coll *x, t_floatarg f)
{
    int ndx;
    if (loud_checkint((t_pd *)x, f, &ndx, gensym("min")))
    {
	t_collelem *found;
	if (ndx > 0)
	    ndx--;
	/* LATER consider complaining: */
	else if (ndx < 0)
	    return;  /* CHECKED silently rejected */
	/* else CHECKED silently defaults to 1 */
	if (found = coll_firsttyped(x, ndx, A_FLOAT))
	{
	    t_float result = found->e_data[ndx].a_w.w_float;
	    t_collelem *ep;
	    for (ep = found->e_next; ep; ep = ep->e_next)
	    {
		if (ep->e_size > ndx &&
		    ep->e_data[ndx].a_type == A_FLOAT &&
		    ep->e_data[ndx].a_w.w_float < result)
		{
		    found = ep;
		    result = ep->e_data[ndx].a_w.w_float;
		}
	    }
	    coll_keyoutput(x, found);
	    outlet_float(((t_object *)x)->ob_outlet, result);
	}
    }
}

static void coll_max(t_coll *x, t_floatarg f)
{
    int ndx;
    if (loud_checkint((t_pd *)x, f, &ndx, gensym("max")))
    {
	t_collelem *found;
	if (ndx > 0)
	    ndx--;
	/* LATER consider complaining: */
	else if (ndx < 0)
	    return;  /* CHECKED silently rejected */
	/* else CHECKED silently defaults to 1 */
	if (found = coll_firsttyped(x, ndx, A_FLOAT))
	{
	    t_float result = found->e_data[ndx].a_w.w_float;
	    t_collelem *ep;
	    for (ep = found->e_next; ep; ep = ep->e_next)
	    {
		if (ep->e_size > ndx &&
		    ep->e_data[ndx].a_type == A_FLOAT &&
		    ep->e_data[ndx].a_w.w_float > result)
		{
		    found = ep;
		    result = ep->e_data[ndx].a_w.w_float;
		}
	    }
	    coll_keyoutput(x, found);
	    outlet_float(((t_object *)x)->ob_outlet, result);
	}
    }
}

static void coll_refer(t_coll *x, t_symbol *s)
{
    if (!coll_rebind(x, s))
    {
	/* LATER consider complaining */
    }
}

static void coll_flags(t_coll *x, t_float f1, t_float f2)
{
    int i1;
    if (loud_checkint((t_pd *)x, f1, &i1, gensym("flags")))
    {
	t_collcommon *cc = x->x_common;
	cc->c_embedflag = (i1 != 0);
    }
}

static void coll_read(t_coll *x, t_symbol *s)
{
    t_collcommon *cc = x->x_common;
    if (s && s != &s_)
	collcommon_doread(cc, s, x->x_canvas);
    else
	hammerpanel_open(cc->c_filehandle, 0);
}

static void coll_write(t_coll *x, t_symbol *s)
{
    t_collcommon *cc = x->x_common;
    if (s && s != &s_)
	collcommon_dowrite(cc, s, x->x_canvas);
    else
	hammerpanel_save(cc->c_filehandle, 0, 0);  /* CHECKED no default name */
}

static void coll_readagain(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    if (cc->c_filename)
	collcommon_doread(cc, 0, 0);
    else
	hammerpanel_open(cc->c_filehandle, 0);
}

static void coll_writeagain(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    if (cc->c_filename)
	collcommon_dowrite(cc, 0, 0);
    else
	hammerpanel_save(cc->c_filehandle, 0, 0);  /* CHECKED no default name */
}

static void coll_filetype(t_coll *x, t_symbol *s)
{
    /* dummy */
}

static void coll_dump(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next)
    {
	coll_keyoutput(x, ep);
	if (cc->c_selfmodified)
	    break;
	coll_dooutput(x, ep->e_size, ep->e_data);
	/* FIXME dooutput() may invalidate ep as well as keyoutput()... */
    }
    outlet_bang(x->x_dumpbangout);
}

static void coll_open(t_coll *x)
{
    t_collcommon *cc = x->x_common;
    t_binbuf *bb = binbuf_new();
    int i, natoms, newline;
    t_atom *ap;
    char buf[MAXPDSTRING];
    hammereditor_open(cc->c_filehandle,
		      (x->x_name ? x->x_name->s_name : "Untitled"), "coll");
    collcommon_tobinbuf(cc, bb);
    natoms = binbuf_getnatom(bb);
    ap = binbuf_getvec(bb);
    newline = 1;
    while (natoms--)
    {
	char *ptr = buf;
    	if (ap->a_type != A_SEMI && ap->a_type != A_COMMA && !newline)
	    *ptr++ = ' ';
    	atom_string(ap, ptr, MAXPDSTRING);
    	if (ap->a_type == A_SEMI)
	{
	    strcat(buf, "\n");
	    newline = 1;
	}
	else newline = 0;
	hammereditor_append(cc->c_filehandle, buf);
	ap++;
    }
    hammereditor_setdirty(cc->c_filehandle, 0);
    binbuf_free(bb);
}

/* CHECKED if there was any editing, both close window and 'wclose'
   ask and replace the contents.  LATER debug. */
static void coll_wclose(t_coll *x)
{
    hammereditor_close(x->x_common->c_filehandle, 1);
}

static void coll_click(t_coll *x, t_floatarg xpos, t_floatarg ypos,
		       t_floatarg shift, t_floatarg ctrl, t_floatarg alt)
{
    coll_open(x);
}

#ifdef COLL_DEBUG
static void collelem_post(t_collelem *ep)
{
    if (ep->e_hasnumkey && ep->e_symkey)
	loudbug_startpost("%d %s:", ep->e_numkey, ep->e_symkey->s_name);
    else if (ep->e_hasnumkey)
	loudbug_startpost("%d:", ep->e_numkey);
    else if (ep->e_symkey)
	loudbug_startpost("%s:", ep->e_symkey->s_name);
    else loudbug_bug("collcommon_post");
    loudbug_postatom(ep->e_size, ep->e_data);
    loudbug_endpost();
}

static void collcommon_post(t_collcommon *cc)
{
    t_collelem *ep;
    for (ep = cc->c_first; ep; ep = ep->e_next) collelem_post(ep);
}

static void coll_debug(t_coll *x, t_floatarg f)
{
    t_collcommon *cc = coll_checkcommon(x);
    if (cc)
    {
	t_coll *x1 = cc->c_refs;
	t_collelem *ep, *last;
	int i = 0;
	while (x1) i++, x1 = x1->x_next;
	loudbug_post("refcount %d", i);
	for (ep = cc->c_first, last = 0; ep; ep = ep->e_next) last = ep;
	if (last != cc->c_last) loudbug_bug("coll_debug: last element");
	collcommon_post(cc);
    }
}
#endif

static void coll_free(t_coll *x)
{
    hammerfile_free(x->x_filehandle);
    coll_unbind(x);
}

static void *coll_new(t_symbol *s)
{
    t_coll *x = (t_coll *)pd_new(coll_class);
    x->x_canvas = canvas_getcurrent();
    outlet_new((t_object *)x, &s_);
    x->x_keyout = outlet_new((t_object *)x, &s_);
    x->x_filebangout = outlet_new((t_object *)x, &s_bang);
    x->x_dumpbangout = outlet_new((t_object *)x, &s_bang);
    x->x_filehandle = hammerfile_new((t_pd *)x, coll_embedhook, 0, 0, 0);
    coll_bind(x, s);
    return (x);
}

void coll_setup(void)
{
    coll_class = class_new(gensym("coll"),
			   (t_newmethod)coll_new,
			   (t_method)coll_free,
			   sizeof(t_coll), 0, A_DEFSYM, 0);
    class_addbang(coll_class, coll_next);
    class_addfloat(coll_class, coll_float);
    class_addsymbol(coll_class, coll_symbol);
    class_addlist(coll_class, coll_list);
    class_addanything(coll_class, coll_anything);
    class_addmethod(coll_class, (t_method)coll_store,
		    gensym("store"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_nstore,
		    gensym("nstore"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_insert,
		    gensym("insert"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_remove,
		    gensym("remove"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_delete,
		    gensym("delete"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_assoc,
		    gensym("assoc"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_deassoc,
		    gensym("deassoc"), A_SYMBOL, A_FLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_subsym,
		    gensym("subsym"), A_SYMBOL, A_SYMBOL, 0);
    class_addmethod(coll_class, (t_method)coll_renumber,
		    gensym("renumber"), A_DEFFLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_merge,
		    gensym("merge"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_sub,
		    gensym("sub"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_sub,
		    gensym("nsub"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_clear,
		    gensym("clear"), 0);
    class_addmethod(coll_class, (t_method)coll_sort,
		    gensym("sort"), A_FLOAT, A_DEFFLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_swap,
		    gensym("swap"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_next,
		    gensym("next"), 0);
    class_addmethod(coll_class, (t_method)coll_prev,
		    gensym("prev"), 0);
    class_addmethod(coll_class, (t_method)coll_end,
		    gensym("end"), 0);
    class_addmethod(coll_class, (t_method)coll_goto,
		    gensym("goto"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_nth,
		    gensym("nth"), A_GIMME, 0);
    class_addmethod(coll_class, (t_method)coll_length,
		    gensym("length"), 0);
    class_addmethod(coll_class, (t_method)coll_min,
		    gensym("min"), A_DEFFLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_max,
		    gensym("max"), A_DEFFLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_refer,
		    gensym("refer"), A_SYMBOL, 0);
    class_addmethod(coll_class, (t_method)coll_flags,
		    gensym("flags"), A_FLOAT, A_FLOAT, 0);
    class_addmethod(coll_class, (t_method)coll_read,
		    gensym("read"), A_DEFSYM, 0);
    class_addmethod(coll_class, (t_method)coll_write,
		    gensym("write"), A_DEFSYM, 0);
    class_addmethod(coll_class, (t_method)coll_readagain,
		    gensym("readagain"), 0);
    class_addmethod(coll_class, (t_method)coll_writeagain,
		    gensym("writeagain"), 0);
    class_addmethod(coll_class, (t_method)coll_filetype,
		    gensym("filetype"), A_SYMBOL, 0);
    class_addmethod(coll_class, (t_method)coll_dump,
		    gensym("dump"), 0);
    class_addmethod(coll_class, (t_method)coll_open,
		    gensym("open"), 0);
    class_addmethod(coll_class, (t_method)coll_wclose,
		    gensym("wclose"), 0);
    class_addmethod(coll_class, (t_method)coll_click,
		    gensym("click"),
		    A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, A_FLOAT, 0);
#ifdef COLL_DEBUG
    class_addmethod(coll_class, (t_method)coll_debug,
		    gensym("debug"), A_DEFFLOAT, 0);
#endif
    hammerfile_setup(coll_class, 1);
    collcommon_class = class_new(gensym("coll"), 0, 0,
				 sizeof(t_collcommon), CLASS_PD, 0);
    /* this call is a nop (collcommon does not embed, and the hammerfile
       class itself has been already set up above), but it is better to
       have it around, just in case... */
    hammerfile_setup(collcommon_class, 0);
}
