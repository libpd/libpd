/* Copyright (c) 2007 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>
#include "m_pd.h"
#include "common/loud.h"
#include "common/grow.h"
#include "sickle/sic.h"
#include "riddle.h"
#include "riddleguts.h"

/* obj_starttraverseoutlet, obj_nexttraverseoutlet, obj_noutlets,
   obj_nsiginlets, obj_nsigoutlets, obj_siginletindex, obj_sigoutletindex,
   obj_issignalinlet, obj_issignaloutlet */
#include "m_imp.h"

/* struct _glist, canvas_class, vinlet_class */
#include "g_canvas.h"

/* it is horrible, but we need x_canvas and x_parentoutlet and o_next for
   pushing through an outlet~, which is here for the completeness of the tests;
   LATER remove from library version, unless there is an API way to do it... */

/* from g_io.c */
typedef struct _rdvoutlet
{
    t_object x_obj;
    t_canvas *x_canvas;
    t_outlet *x_parentoutlet;
    /* ... */
} t_rdvoutlet;

/* from m_obj.c */
typedef struct _rdoutlet
{
    t_object *o_owner;
    struct _rdoutlet *o_next;
    /* ... */
} t_rdoutlet;

#define RIDDLE_DEBUG

struct _rdprivate
{
    t_outconnect  *pr_oc;
};

struct _rdsource
{
    t_riddle    *so_riddle;
    t_rdremote  *so_remote;
    int          so_sourcecount;
    t_symbol    *so_pattern;
    t_symbol    *so_newpattern;
    int          so_block;  /* if non-zero pattern: largest expected block */
    int          so_newblock;
    int          so_flags;
};

struct _rdsink
{
    t_riddle       *si_riddle;
    int             si_outno;
    t_symbol       *si_pattern;
    int             si_block;  /* if non-zero pattern: largest expected block */
    int             si_flags;
    t_atom          si_outbuf[4];  /* siginno, pattern, block, flags */
    t_rdfeedchain  *si_feedchain;
    int             si_isready;
};

/* these are filled in riddle_setup() */
static t_symbol *rdps__reblock = 0;
static t_symbol *rdps__ = 0;

void riddlebug_post(t_riddle *rd, char *pfx, char *fmt, ...)
{
    char buf[MAXPDSTRING];
    va_list ap;
    if (fmt)
    {
	va_start(ap, fmt);
	vsnprintf(buf, MAXPDSTRING-1, fmt, ap);
	va_end(ap);
	fprintf(stderr, "%s \"%s\" (%x): %s\n",
		pfx, class_getname(*(t_pd *)rd), (int)rd, buf);
    }
    else fprintf(stderr, "%s \"%s\" (%x)\n",
		 pfx, class_getname(*(t_pd *)rd), (int)rd);
#ifdef MSW
    fflush(stderr);
#endif
}

int riddle_getsr(t_riddle *rd)
{
    return (rd->rd_graphsr);
}

int riddle_getgraphblock(t_riddle *rd)
{
    return (rd->rd_graphblock);
}

int riddle_getsourceblock(t_riddle *rd, int siginno)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
    {
	loudbug_bug("riddle_getsourceblock");
	return (0);
    }
    else
    {
	t_rdsource *so = (siginno >= 0 ?
			  rd->rd_inslots + siginno :
			  rd->rd_remoteslots - ++siginno);
	return (so->so_pattern ? 0 : so->so_block);  /* FIXME disable? */
    }
}

t_symbol *riddle_getsourcelayout(t_riddle *rd, int siginno, int *maxblockp)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
    {
	loudbug_bug("riddle_getsourcelayout");
	return (0);
    }
    else
    {
	t_rdsource *so = (siginno >= 0 ?
			  rd->rd_inslots + siginno :
			  rd->rd_remoteslots - ++siginno);
	if (maxblockp)
	    *maxblockp = so->so_block;
	return (so->so_pattern);
    }
}

int riddle_getsourceflags(t_riddle *rd, int siginno)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
    {
	loudbug_bug("riddle_getsourceflags");
	return (0);
    }
    else
    {
	t_rdsource *so = (siginno >= 0 ?
			  rd->rd_inslots + siginno :
			  rd->rd_remoteslots - ++siginno);
	return (so->so_flags);
    }
}

/* LATER rethink the remote case */
void riddle_setsourceblock(t_riddle *rd, int siginno, int newblock)
{
    int slotno = (siginno < 0 ? rd->rd_nsiginlets - siginno - 1 : siginno);
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setsourceblock", "%d (%d) %d",
		   siginno, slotno, newblock);
#endif
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_setsourceblock");
    else if (newblock <= 0)
	loud_error((t_pd *)rd,
		   "invalid source block on inlet %d: %d", siginno, newblock);
    else
    {
	t_rdsource *so = rd->rd_inslots + slotno;
	/* LATER if (so->so_newpattern) complain */
	if (newblock == so->so_newblock)
	    so->so_sourcecount++;
	else if (so->so_sourcecount > 0)
	    loud_error((t_pd *)rd,
		       "source block mismatch on inlet %d: %d != %d",
		       siginno, newblock, so->so_newblock);
	else
	{
	    so->so_newblock = newblock;
	    so->so_sourcecount = 1;
	}
    }
}

#define RDLAYOUT_MAXNVECTORS  32

/* apart from normalization, this is used only as a sanity check; patterns
   are never interpreted, they just have to match (after normalization) */
static t_symbol *riddle_validatepattern(t_symbol *pattern)
{
    char *p = pattern->s_name, lc, uc;
    switch (*p)
    {
    case 'a':
    case 'A':
	lc = 'b'; uc = 'A'; break;
    case 'b':
	lc = 'c'; uc = 'B'; break;
    default:
	lc = 0;
    }
    if (lc)
    {
	/* we require at least one vector for each size element */
	int vused[RDLAYOUT_MAXNVECTORS], i;
	for (i = 0; i < RDLAYOUT_MAXNVECTORS; i++)
	    vused[i] = 0;
	if (*p == 'A')
	    vused[0] = 1;
	for (p++; *p; p++)
	{
	    if (*p == lc)
	    {
		if (lc - 'a' < RDLAYOUT_MAXNVECTORS)
		    lc++, uc++;
		else
		    break;
	    }
	    else if (*p >= 'A' && *p <= uc)
		vused[*p - 'A'] = 1;
	    else
		break;
	}
	if (!*p)
	{
	    for (i = 0; i < lc - 'a'; i++)
		if (!vused[i])
		    break;
	    if (i == lc - 'a')
	    {
		if (*pattern->s_name == 'a')  /* normalization */
		    pattern = gensym(pattern->s_name + 1);
		return (pattern);
	    }
	}
    }
    loudbug_bug("riddle_validatepattern");
    return (0);
}

void riddle_setsourcelayout(t_riddle *rd, int siginno,
			    t_symbol *newpattern, int maxblock)
{
    int slotno = (siginno < 0 ? rd->rd_nsiginlets - siginno - 1 : siginno);
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setsourcelayout", "%d (%d) %s %d",
		   siginno, slotno, newpattern->s_name, maxblock);
#endif
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_setsourcelayout");
    else
    {
	t_rdsource *so = rd->rd_inslots + slotno;
	if (newpattern == so->so_newpattern)
	    so->so_sourcecount++;
	else if (so->so_sourcecount > 0)
	{
	    if (so->so_newpattern)
		loud_error((t_pd *)rd,
			   "source layout mismatch on inlet %d: %s != %s",
			   siginno, newpattern->s_name,
			   so->so_newpattern->s_name);
	    else
		loud_error((t_pd *)rd,
			   "source layout/block mismatch on inlet %d");
	}
	else if (newpattern = riddle_validatepattern(newpattern))
	{
	    so->so_newpattern = newpattern;
	    if (maxblock)
	    {
		if (maxblock > so->so_newblock)
		    so->so_newblock = maxblock;
	    }
	    else so->so_newblock = rd->rd_graphblock;
	    so->so_sourcecount = 1;
	}
    }
}

void riddle_setsourceflags(t_riddle *rd, int siginno, int flags)
{
    int slotno = (siginno < 0 ? rd->rd_nsiginlets - siginno - 1 : siginno);
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setsourceflags", "%d (%d) %d",
		   siginno, slotno, flags);
#endif
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_setsourceflags");
    else
    {
	t_rdsource *so = rd->rd_inslots + slotno;
	so->so_flags = flags;
    }
}

void riddle_setoutblock(t_riddle *rd, int sigoutno, int block)
{
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setoutblock", "%d %d", sigoutno, block);
#endif
    if (sigoutno < 0 || sigoutno >= rd->rd_nsigoutlets)
	loudbug_bug("riddle_setoutblock");
    else
    {
	t_rdsink *si = rd->rd_outslots + sigoutno;
	si->si_pattern = 0;
	si->si_block = block;
	si->si_outbuf[1].a_w.w_symbol = rdps__;
	si->si_outbuf[2].a_w.w_float = (t_float)block;
	si->si_isready = 1;
    }
}

void riddle_setoutlayout(t_riddle *rd, int sigoutno,
			 t_symbol *pattern, int maxblock)
{
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setoutlayout", "%d %s %d",
		   sigoutno, pattern->s_name, maxblock);
#endif
    if (sigoutno < 0 || sigoutno >= rd->rd_nsigoutlets)
	loudbug_bug("riddle_setoutlayout");
    else if (pattern = riddle_validatepattern(pattern))
    {
	t_rdsink *si = rd->rd_outslots + sigoutno;
	if (maxblock <= 0)
	    maxblock = rd->rd_graphblock;
	si->si_pattern = pattern;
	si->si_block = maxblock;
	si->si_outbuf[1].a_w.w_symbol = pattern;
	si->si_outbuf[2].a_w.w_float = (t_float)maxblock;
	si->si_isready = 1;
    }
}

void riddle_setoutflags(t_riddle *rd, int sigoutno, int flags)
{
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "setoutflags", "%d %d", sigoutno, flags);
#endif
    if (sigoutno < 0 || sigoutno >= rd->rd_nsigoutlets)
	loudbug_bug("riddle_setoutflags");
    else
    {
	t_rdsink *si = rd->rd_outslots + sigoutno;
	si->si_flags = flags;
	si->si_outbuf[3].a_w.w_float = (t_float)flags;
    }
}

int riddle_checksourceblock(t_riddle *rd, int siginno, int reqblock)
{
    int block = riddle_getsourceblock(rd, siginno);
    if (block == reqblock)
	return (1);
    else
    {
	if (!rd->rd_wasdisabled && rd->rd_inslots[siginno].so_sourcecount)
	    loud_error((t_pd *)rd,
		       "invalid source block on inlet %d: %d (%d expected)",
		       siginno, block, reqblock);
	rd->rd_disabled = 1;
	return (0);
    }
}

int riddle_checksourcelayout(t_riddle *rd, int siginno,
			     t_symbol *reqpattern, int *maxblockp)
{
    t_symbol *pattern = riddle_getsourcelayout(rd, siginno, maxblockp);
    if (reqpattern == pattern ||
	riddle_validatepattern(reqpattern) == pattern)
	return (1);
    else
    {
	if (!rd->rd_wasdisabled && rd->rd_inslots[siginno].so_sourcecount)
	{
	    if (pattern)
		loud_error((t_pd *)rd,
			   "wrong source layout on inlet %d: %s (%s expected)",
			   siginno, pattern->s_name, reqpattern->s_name);
	    else
		loud_error((t_pd *)rd,
			   "invalid source on inlet %d: layout %s expected",
			   siginno, reqpattern->s_name);
	}
	rd->rd_disabled = 1;
	return (0);
    }
}

int riddle_checkanysource(t_riddle *rd, int siginno)
{
    if (siginno >= rd->rd_nsiginlets || -siginno > rd->rd_nremoteslots)
	loudbug_bug("riddle_checkanysource");
    else
    {
	t_rdsource *so = (siginno >= 0 ?
			  rd->rd_inslots + siginno :
			  rd->rd_remoteslots - ++siginno);
	if (so->so_sourcecount > 0)
	    return (1);
    }
    rd->rd_disabled = 1;
    return (0);
}

int riddle_isdisabled(t_riddle *rd)
{
    return (rd->rd_disabled);
}

void riddle_disable(t_riddle *rd)
{
    /* FIXME allow calling from the dsp routine (mute then) */
    rd->rd_disabled = 1;
}

t_rdfeedchain *riddle_getfeedchain(t_riddle *rd, int sigoutno)
{
    if (sigoutno < 0 || sigoutno >= rd->rd_nsigoutlets)
    {
	loudbug_bug("riddle_getfeedchain 1");
	return (0);
    }
    else
    {
	t_rdsink *si = rd->rd_outslots + sigoutno;
	if (si->si_outno >= 0)
	{
	    /* LATER update ch_outno */
	    return (si->si_feedchain);
	}
	else
	{
	    loudbug_bug("riddle_getfeedchain 2");
	    return (0);
	}
    }
}

/* ensures that sinks match signal outlets -- this is needed in the constructor,
   but is called before each push, perhaps too defensively... LATER rethink */
static int riddle_validatesinks(t_riddle *rd)
{
    t_object *x = (t_object *)rd;
    int sigoutno, outno, nouts = obj_noutlets(x);
    for (sigoutno = 0, outno = 0; outno < nouts; outno++)
    {
	if (obj_issignaloutlet(x, outno))
	{
	    if (sigoutno < rd->rd_nsigoutlets)
	    {
		if (rd->rd_outslots[sigoutno].si_outno != outno)
		{
		    if (rd->rd_outslots[sigoutno].si_outno < 0)
			rd->rd_outslots[sigoutno].si_outno = outno;
		    else
		    {
			loudbug_bug("riddle_validatesinks 1");
			return (0);
		    }
		}
	    }
	    else
	    {
		loudbug_bug("riddle_validatesinks 2");
		/* LATER grow */
		return (0);
	    }
	    sigoutno++;
	}
    }
    if (sigoutno < rd->rd_nsigoutlets)
    {
	loudbug_bug("riddle_validatesinks 3");
	/* LATER shrink */
	return (0);
    }
    return (1);
}

t_canvas *riddle_nextgraph(t_riddle *rd)
{
    while (rd->rd_private->pr_oc)
    {
	t_object *dst;
	t_inlet *ip;
	int inno;
	rd->rd_private->pr_oc =
	    obj_nexttraverseoutlet(rd->rd_private->pr_oc, &dst, &ip, &inno);
	if (dst)
	{
	    int siginno = obj_siginletindex(dst, inno);
	    if (siginno < 0)
	    {
		/* should not happen, LATER rethink */
		break;
	    }
	    else if (pd_class((t_pd *)dst) != canvas_class)
	    {
		loud_error((t_pd *)rd, "invalid connection (not to a canvas)");
		break;
	    }
	    else return ((t_canvas *)dst);
	}
    }
    return (0);
}

t_canvas *riddle_firstgraph(t_riddle *rd, int outno)
{
    t_outlet *op;
    rd->rd_private->pr_oc = obj_starttraverseoutlet((t_object *)rd, &op, outno);
    return (rd->rd_private->pr_oc ? riddle_nextgraph(rd) : 0);
}

static int rdsink_push(t_rdsink *si, t_object *x, int outno)
{
    int result = 1;
    t_outlet *op;
    t_outconnect *oc = obj_starttraverseoutlet(x, &op, outno);
    while (oc)
    {
	t_object *dst;
	t_inlet *ip;
	int inno;
	oc = obj_nexttraverseoutlet(oc, &dst, &ip, &inno);
	if (dst)
	{
	    int siginno = obj_siginletindex(dst, inno);
	    if (siginno < 0)
	    {
		/* should not happen, LATER rethink */
	    }
	    else if (zgetfn((t_pd *)dst, rdps__reblock))
	    {
		si->si_outbuf->a_w.w_float = (t_float)siginno;
		typedmess((t_pd *)dst, rdps__reblock, 4, si->si_outbuf);
	    }
	    else if (pd_class((t_pd *)dst) == canvas_class)
	    {
		t_gobj *ob;
		int i;
		for (i = 0, ob = ((t_canvas *)dst)->gl_list;
		     ob; ob = ob->g_next)
		{
		    if (pd_class((t_pd *)ob) == vinlet_class)
		    {
			if (i == inno)
			    break;
			else
			    i++;
		    }
		}
		if (ob)
		{
#ifdef RIDDLE_DEBUG
		    riddlebug_post(si->si_riddle, "PUSH-SUBCANVAS",
				   "vinlet %d (\"%s\")",
				   inno, class_getname(*(t_pd *)ob));
#endif
		    rdsink_push(si, (t_object *)ob, 0);
		}
		else loudbug_bug("rdsink_push 1");
	    }
	    else if (pd_class((t_pd *)dst) == voutlet_class)
	    {
		t_rdvoutlet *vout = (t_rdvoutlet *)dst;
		if (vout->x_canvas)
		{
		    int n;
		    t_outlet *o;
		    for (o = ((t_object *)vout->x_canvas)->ob_outlet, n = 0;
			 o; o = (t_outlet *)(((t_rdoutlet *)o)->o_next), n++)
			if (o == vout->x_parentoutlet)
			    break;
		    if (o)
		    {
#ifdef RIDDLE_DEBUG
			riddlebug_post(si->si_riddle, "PUSH-OUTLET",
				       "outno %d, graph %x",
				       n, (int)vout->x_canvas);
#endif
			rdsink_push(si, (t_object *)vout->x_canvas, n);
		    }
		    else loudbug_bug("rdsink_push 2");
		}
#ifdef RIDDLE_DEBUG
		else riddlebug_post(si->si_riddle, "PUSH-OUTLET",
				    "void canvas...");
#endif
	    }
	    else
	    {
		char *dstname = class_getname(*(t_pd *)dst);
#ifdef RIDDLE_DEBUG
		riddlebug_post(si->si_riddle, "PUSH-RIDDLESS",
			       "inlet %d (\"%s\")", inno, dstname);
#endif
		if (si->si_flags & RIDDLE_STRICTNESSMASK)
		{
		    if (strcmp(dstname, "print~"))
		    {
			loud_error((t_pd *)x, "not a riddle: \"%s\"", dstname);
			result = 0;
		    }
		}
		else if (!strcmp(dstname, "send~") ||
			 !strcmp(dstname, "throw~"))
		{
		    loud_error((t_pd *)x, "bad destination: \"%s\"", dstname);
		    result = 0;
		}
	    }
	}
    }
    return (result);
}

static void riddle_mute(t_riddle *rd, t_signal **sp)
{
    int i, j, nouts = obj_nsigoutlets((t_object *)rd);
    t_rdsink *si = rd->rd_outslots;
#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "MUTE", 0);
#endif
    if (rd->rd_nsigoutlets != nouts)
    {
	loudbug_bug("riddle_mute");
	riddle_validatesinks(rd);
	if (rd->rd_nsigoutlets != nouts)
	    return;
    }
    i = 0;
    j = obj_nsiginlets((t_object *)rd);
    while (nouts--)
    {
	si->si_pattern = 0;
	si->si_block = sp[j]->s_n;
	si->si_outbuf[1].a_w.w_symbol = rdps__;
	si->si_outbuf[2].a_w.w_float = (t_float)si->si_block;
	si->si_isready = 1;
	dsp_add_zero(sp[j]->s_vec, sp[j]->s_n);
	i++; j++;
	si++;
    }
}

static void riddle_dsp(t_riddle *rd, t_signal **sp)
{
    int failed = 0, unarmed = 1, doreblock = 0;
    int oldgraphsr = rd->rd_graphsr;
    int oldgraphblock = rd->rd_graphblock;
    int inslotno, ninslots = rd->rd_nsiginlets + rd->rd_nremoteslots;
    int outslotno;
    t_rdsource *inslot;
    t_rdsink *outslot;

#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "\nriddle_dsp", 0);
    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
	loudbug_post("%d sources: %d reblocks of %d -> %d",
		     inslotno, inslot->so_sourcecount,
		     inslot->so_block, inslot->so_newblock);
#endif

    rd->rd_graphsr = (int)sp[0]->s_sr;
    rd->rd_graphblock = sp[0]->s_n;

    /* this belongs to step 2., but should precede all "muteandreset" gotos */
    if (rd->rd_wasdisabled = rd->rd_disabled)
    {
	rd->rd_disabled = 0;
	if (rd->rd_blockfn)
	    doreblock = 1;
	else
	{
	    loudbug_bug("riddle_dsp 1");
	    goto muteandreset;
	}
    }

    /* step 1: verify all source slots */

    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
    {
	if (inslot->so_newblock > rd->rd_graphblock)
	{
	    if (inslotno < rd->rd_nsiginlets)
	    {
		loud_error((t_pd *)rd,
			   "inslot %d: source block too large (%d > %d)",
			   inslotno, inslot->so_newblock, rd->rd_graphblock);
		failed = 1;
	    }
	}
	else if (inslot->so_sourcecount <= 0)
	{
	    if (inslotno < rd->rd_nsiginlets)
	    {
		/* bash unconfirmed declarations to graphblock */
		inslot->so_newpattern = 0;
		inslot->so_newblock = rd->rd_graphblock;
	    }
	    else if (inslot->so_remote)
	    {
		if (rdremote_getwriter(inslot->so_remote))
		{
		    loud_warning((t_pd *)rd, 0, "misplaced buffer reader...");
		    riddle_updatedsp();
		    failed = 1;  /* LATER rethink */
		}
		else
		{
		    loud_warning((t_pd *)rd, 0, "orphaned buffer reader");

		    /* remote slots preserve unconfirmed declarations */
		    inslot->so_newpattern = inslot->so_pattern;
		    if (inslot->so_block > 0)
			inslot->so_newblock = inslot->so_block;
		    else
			inslot->so_newblock = rd->rd_graphblock;
		}
	    }
	    else loudbug_bug("riddle_dsp 2");
	}
	else if (inslot->so_newblock <= 0)  /* should not happen */
	{
	    loudbug_bug("riddle_dsp 3");
	    failed = 1;
	}
    }
    if (failed)
	goto muteandreset;

    /* step 2: determine outslot sizes/layouts -- blockfn fires on the very
       first call to riddle_dsp(), and then after any change of block or sr,
       and each time the object is disabled... LATER reconsider the pros
       and cons of performing the reblocking during every dsp call */

    /* 2a: was there any change of inslot size/layout or graph block/sr? */
    if (!doreblock && rd->rd_blockfn)
    {
	if (rd->rd_graphsr != oldgraphsr ||
	    rd->rd_graphblock != oldgraphblock)
	    doreblock = 1;
	else for (inslotno = 0, inslot = rd->rd_inslots;
		  inslotno < ninslots; inslotno++, inslot++)
	{
	    if (inslot->so_newpattern != inslot->so_pattern ||
		inslot->so_newblock != inslot->so_block)
	    {
		doreblock = 1;
		break;
	    }
	}
    }

    /* 2b: update the inslots, reset the outslots */
    if (doreblock || !rd->rd_blockfn)
    {
	for (inslotno = 0, inslot = rd->rd_inslots;
	     inslotno < ninslots; inslotno++, inslot++)
	{
	    inslot->so_pattern = inslot->so_newpattern;
	    inslot->so_block = inslot->so_newblock;
	}
	for (outslotno = 0, outslot = rd->rd_outslots;
	     outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
	{
	    outslot->si_pattern = 0;
	    outslot->si_block = 0;
	    outslot->si_isready = 0;
	}
    }

    /* 2c: call the instance-specific method which redeclares the outslots */
    if (doreblock)
    {
#ifdef RIDDLE_DEBUG
	riddlebug_post(rd, "REBLOCK", 0);
#endif
	rd->rd_blockfn(rd);
	if (rd->rd_disabled)
	    goto muteandreset;
    }

    /* 2d: assign defaults to undeclared outslots */
    for (outslotno = 0, outslot = rd->rd_outslots;
	 outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
    {
	if (outslot->si_block < 0)
	{
	    loudbug_bug("riddle_dsp 4");
	    failed = 1;
	}
	else if (outslot->si_block == 0)
	    outslot->si_block = rd->rd_graphblock;
    }
    /* LATER think about not redeclared remote writers */
    if (failed)
	goto muteandreset;

    /* step 3: transfer outslot declarations down to destination objects */

#ifdef RIDDLE_DEBUG
    riddlebug_post(rd, "PUSH", 0);
#endif
    if (riddle_validatesinks(rd))
    {
	for (outslotno = 0, outslot = rd->rd_outslots;
	     outslotno < rd->rd_nsigoutlets; outslotno++, outslot++)
	    if (outslot->si_isready &&
		rdsink_push(outslot, (t_object *)rd, outslot->si_outno) == 0)
		failed = 1;
    }
    else failed = 1;
    /* remote declarations are propagated directly from within
       rdremote_setoutblock/layout(), cf. rdremote_pushblock/layout() */
    if (failed)
	goto muteandreset;


    /* step 4: call the wrappee */

    if (rd->rd_dspfn)
    {
	rd->rd_dspfn(rd, sp);
	unarmed = 0;
    }
    else loudbug_bug("riddle_dsp 5");

    /* step 5: mute if disabled, then reset the inslots */

muteandreset:
    if (unarmed)
    {
	rd->rd_disabled = 1;
	riddle_mute(rd, sp);
    }
    for (inslotno = 0, inslot = rd->rd_inslots;
	 inslotno < ninslots; inslotno++, inslot++)
    {
	inslot->so_newpattern = 0;
	inslot->so_newblock = 0;
	inslot->so_sourcecount = 0;
    }
}

static void riddle__reblock(t_riddle *rd, t_symbol *pattern,
			    t_floatarg f1, t_floatarg f2, t_floatarg f3)
{
    riddle_setsourceflags(rd, (int)f1, (int)f3);
    if (pattern == rdps__)
	riddle_setsourceblock(rd, (int)f1, (int)f2);
    else if (pattern)
	riddle_setsourcelayout(rd, (int)f1, pattern, (int)f2);
    else
	loud_error((t_pd *)rd, "bad arguments to '_reblock'");
}

static void riddle_free(t_riddle *rd)
{
    t_gotfn freefn = zgetfn((t_pd *)rd, gensym("_free"));
    if (freefn)
	freefn(rd);

    if (rd->rd_private)
	freebytes(rd->rd_private, sizeof(*rd->rd_private));

    if (rd->rd_inslots)
    {
	int nslots = rd->rd_nsiginlets + rd->rd_nremoteslots;
	freebytes(rd->rd_inslots, nslots * sizeof(*rd->rd_inslots));
    }

    if (rd->rd_outslots)
    {
	t_rdsink *si;
	int i;
	for (i = 0, si = rd->rd_outslots; i < rd->rd_nsigoutlets; i++, si++)
	    if (si->si_feedchain)
		rdfeedchain_free(si->si_feedchain);
	freebytes(rd->rd_outslots,
		  rd->rd_nsigoutlets * sizeof(*rd->rd_outslots));
    }

    rdremote_freeports(rd->rd_remoteports);

    if (rd->rd_idlepicker)
	rdpicker_detach(rd->rd_idlepicker, rd);
}

typedef t_pd *(*t_newgimme)(t_symbol *s, int argc, t_atom *argv);

static void *riddle_new(t_symbol *s, int ac, t_atom *av)
{
    /* IFBUILTIN remove: this is a bad hack */
    t_pd *en = riddle_getenvironment();
    t_newgimme newfn = (t_newgimme)zgetfn(en, s);
    if (!newfn)
    {
	loudbug_bug("riddle_new 1");
	return (0);
    }
    else
    {
	t_riddle *rd = (t_riddle *)newfn(s, ac, av);
	int i, nslots;
	t_rdsource *inslot;
	t_rdsink *outslot;
	t_rdremote *re;
	if (!rd)
	    return (0);

	rd->rd_private = getbytes(sizeof(*rd->rd_private));

	rd->rd_disabled = 0;
	rd->rd_wasdisabled = 0;
	rd->rd_blockfn = (t_rdblockfn)zgetfn((t_pd *)rd, gensym("dspblock"));
	rd->rd_dspfn = (t_rddspfn)zgetfn((t_pd *)rd, gensym("_dsp"));
	if (!rd->rd_dspfn)
	    loudbug_bug("riddle_new 2");

	rd->rd_graphsr = (int)sys_getsr();
	rd->rd_graphblock = sys_getblksize();
	rd->rd_nsiginlets = obj_nsiginlets((t_object *)rd);
	rd->rd_nsigoutlets = obj_nsigoutlets((t_object *)rd);

	/* currently, rd_nremoteslots is incremented in rdbuffer_newreader(),
	   which relies on calloc in pd_new(), LATER rethink */

	nslots = rd->rd_nsiginlets + rd->rd_nremoteslots;
	rd->rd_inslots = getbytes(nslots * sizeof(*rd->rd_inslots));
	for (i = 0, inslot = rd->rd_inslots; i < nslots; i++, inslot++)
	{
	    inslot->so_riddle = rd;
	    inslot->so_remote = 0;
	    inslot->so_sourcecount = 0;
	    inslot->so_pattern = 0;
	    inslot->so_newpattern = 0;
	    inslot->so_block = 0;
	    inslot->so_newblock = 0;
	    inslot->so_flags = 0;
	}
	rd->rd_remoteslots = rd->rd_inslots + rd->rd_nsiginlets;

	for (i = 0, inslot = rd->rd_remoteslots, re = rd->rd_remoteports;
	     i < rd->rd_nremoteslots; i++, inslot++)
	{
	    if (re = rdremote_nextreader(re))
		inslot->so_remote = re;
	    else
	    {
		loudbug_bug("riddle_new 3");
		break;  /* FIXME this is fatal */
	    }
	}

	rd->rd_outslots =
	    getbytes(rd->rd_nsigoutlets * sizeof(*rd->rd_outslots));
	for (i = 0, outslot = rd->rd_outslots;
	     i < rd->rd_nsigoutlets; i++, outslot++)
	{
	    outslot->si_riddle = rd;
	    outslot->si_outno = -1;
	    outslot->si_pattern = 0;
	    outslot->si_block = 0;
	    outslot->si_flags = 0;
	    outslot->si_outbuf[0].a_type = A_FLOAT;
	    outslot->si_outbuf[1].a_type = A_SYMBOL;
	    outslot->si_outbuf[1].a_w.w_symbol = rdps__;
	    outslot->si_outbuf[2].a_type = A_FLOAT;
	    outslot->si_outbuf[3].a_type = A_FLOAT;
	    outslot->si_outbuf[3].a_w.w_float = 0.;
	    outslot->si_feedchain = 0;
	    outslot->si_isready = 0;
	}

	riddle_validatesinks(rd);

	for (i = 0, outslot = rd->rd_outslots;
	     i < rd->rd_nsigoutlets; i++, outslot++)
	    if (outslot->si_outno >= 0)
		outslot->si_feedchain = rdfeedchain_new(outslot->si_outno);

	rd->rd_idlepicker = rdpicker_attach(rd, gensym("_idle"));

	return (rd);
    }
}

/* IFBUILTIN remove: classes would use explicit class_addmethod calls */
/* obligatory: newfn, dspfn */
/* optional: freefn, blockfn, floatfn */
t_class *riddle_setup(t_symbol *name, t_newmethod newfn, t_method freefn,
		      size_t sz, t_method floatfn,
		      t_rdblockfn blockfn, t_rddspfn dspfn)
{
    t_class *c = class_new(name, (t_newmethod)riddle_new,
			   (t_method)riddle_free, sz, 0, A_GIMME, 0);

    /* IFBUILTIN remove: this is a bad hack */
    t_pd *en = riddle_getenvironment();
    class_addmethod(*en, (t_method)newfn, name, 0);

    if (strlen(name->s_name) < 60)
    {
	char rdstr[64];
	sprintf(rdstr, "rd.%s", name->s_name);
	class_addcreator((t_newmethod)riddle_new, gensym(rdstr), A_GIMME, 0);
	class_addmethod(*en, (t_method)newfn, gensym(rdstr), 0);
    }

    rdps__reblock = gensym("_reblock");
    rdps__ = gensym("_");

    sic_setup(c, riddle_dsp, floatfn);
    if (blockfn)
	class_addmethod(c, (t_method)blockfn, gensym("dspblock"), 0);
    /* IFBUILTIN "_dsp" -> "dsp" */
    class_addmethod(c, (t_method)dspfn, gensym("_dsp"), 0);
    /* IFBUILTIN remove these two */
    class_addmethod(c, (t_method)newfn, gensym("_new"), 0);
    if (freefn)
	class_addmethod(c, (t_method)freefn, gensym("_free"), 0);
    class_addmethod(c, (t_method)riddle__reblock,
		    rdps__reblock, A_FLOAT, A_SYMBOL, A_FLOAT, A_FLOAT, 0);
    return (c);
}

/* Fills an array of band sizes, in bins, which partition an nbins-point power
   spectrum into nbands or less ERB bands (nbands is a requested number of
   bands, the actual number is returned).  The buffer is then zero-terminated
   (and zero-padded if necessary), so its size has to be at least nbands+1. */
int riddle_erbfill(int nbands, int *buf, int nbins, int sr)
{
    static double coef = 9.293902;  /* 21.4 / log(10) */
    double df = (double)sr / (double)nbins;
    double fmax = .5 * (nbins + 1) * df;
    double fc = df;
    int i, erbcount = 0, bincount = 0, lastbin = 0;
    int bufsize = nbands + 1;
    while (erbcount < nbands && fc < fmax)
    {
	/* the formula is taken from ~jos/bbt
	   (the results slightly differ from moore-glasberg's demos) */
	double erbnumber = coef * log(.00437 * fc + 1.);
	bincount++;
	if ((int)erbnumber > erbcount)  /* LATER rethink */
	{
	    buf[erbcount++] = bincount - lastbin;
	    lastbin = bincount;
	}
	fc += df;
    }
    for (i = erbcount; i < bufsize; i++)
	buf[i] = 0;
    return (erbcount);
}
