/* Copyright (c) 2003-2005 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#include <stdio.h>
#include <string.h>
#ifdef UNIX
#include <unistd.h>
#endif
#ifdef NT
#include <io.h>
#endif
#include "m_pd.h"
#include "g_canvas.h"
#include "common/loud.h"
#include "common/grow.h"
#include "common/props.h"
#include "scriptlet.h"

#ifdef KRZYSZCZ
//#define SCRIPTLET_DEBUG
#endif

#define SCRIPTLET_INISIZE    1024
#define SCRIPTLET_INIDOTSIZE  256
#define SCRIPTLET_MARGIN       64
#define SCRIPTLET_DOTMARGIN    16
/* cf SOCKSIZE in older versions of t_tkcmd.c, FIXME */
#define SCRIPTLET_MAXPUSH   20000

enum { SCRIPTLET_CVOK, SCRIPTLET_CVUNKNOWN, SCRIPTLET_CVMISSING };

#define VERSLET_MAXPACKAGE  32
#define VERSLET_MAXVERSION  32

typedef struct _verslet
{
    t_pd  *v_owner;
    char   v_package[VERSLET_MAXPACKAGE];
    char   v_version[VERSLET_MAXVERSION];
} t_verslet;

struct _scriptlet
{
    t_pd             *s_owner;
    t_glist          *s_glist;     /* containing glist (empty allowed) */
    t_symbol         *s_rptarget;  /* reply target */
    t_symbol         *s_cbtarget;  /* callback target */
    t_symbol         *s_item;      /* item's name (readable part of its path) */
    t_scriptlet_cvfn  s_cvfn;      /* if empty, passing resolveall is a bug */
    t_canvas         *s_cv;        /* as returned by cvfn */
    int               s_cvstate;
    int    s_locked;  /* append lock, for filtering, when reading from file */
    int    s_size;
    char  *s_buffer;
    char   s_bufini[SCRIPTLET_INISIZE];
    char  *s_head;       /* ptr to the command part of a scriptlet */
    char  *s_tail;
    char   s_separator;  /* current separator, set before a new token */
    int    s_dotsize;
    int    s_dotoffset;
    char  *s_dotbuffer;
    char   s_dotbufini[SCRIPTLET_INIDOTSIZE];
};

static t_canvas *scriptlet_canvasvalidate(t_scriptlet *sp, int visedonly)
{
    t_canvas *cv;
    if (sp->s_cvstate == SCRIPTLET_CVUNKNOWN)
    {
	if (sp->s_cvfn)
	    cv = sp->s_cv = sp->s_cvfn(sp->s_owner);
	else
	{
	    loudbug_bug("scriptlet_canvasvalidate");
	    return (0);
	}
	if (cv && (!visedonly || glist_isvisible(cv)))
	    sp->s_cvstate = SCRIPTLET_CVOK;
	else
	    sp->s_cvstate = SCRIPTLET_CVMISSING;
    }
    else cv = sp->s_cv;
    return (sp->s_cvstate == SCRIPTLET_CVOK ? cv : 0);
}

static int scriptlet_ready(t_scriptlet *sp)
{
    int len = sp->s_tail - sp->s_head;
    if (len > 0 && *sp->s_head && sp->s_cvstate != SCRIPTLET_CVMISSING)
    {
	if (len < SCRIPTLET_MAXPUSH)
	    return (1);
	else
	    loud_error(sp->s_owner,
		       "scriptlet too long to be pushed (%d bytes)", len);
    }
    return (0);
}

static int scriptlet_doappend(t_scriptlet *sp, char *buf)
{
    if (buf && !sp->s_locked)
    {
	int nprefix = sp->s_head - sp->s_buffer;
	int nused = sp->s_tail - sp->s_buffer;
	int newsize = nused + strlen(buf) + SCRIPTLET_MARGIN;
	if (newsize > sp->s_size)
	{
	    int nrequested = newsize;
	    sp->s_buffer = grow_withdata(&nrequested, &nused,
					 &sp->s_size, sp->s_buffer,
					 SCRIPTLET_INISIZE, sp->s_bufini,
					 sizeof(*sp->s_buffer));
	    if (nrequested != newsize)
	    {
		scriptlet_reset(sp);
		return (0);
	    }
	    sp->s_head = sp->s_buffer + nprefix;
	    sp->s_tail = sp->s_buffer + nused;
	}
	if (sp->s_separator && sp->s_tail > sp->s_head)
	    *sp->s_tail++ = sp->s_separator;
	*sp->s_tail = 0;
	strcpy(sp->s_tail, buf);
	sp->s_tail += strlen(sp->s_tail);
    }
    sp->s_separator = 0;
    return (1);
}

static int scriptlet_dotstring(t_scriptlet *sp, char *st)
{
    int len = strlen(st),
	newsize = sp->s_dotoffset + len + SCRIPTLET_DOTMARGIN;
    if (newsize > sp->s_dotsize)
    {
	int nrequested = newsize;
	sp->s_dotbuffer = grow_withdata(&nrequested, &sp->s_dotoffset,
					&sp->s_dotsize, sp->s_dotbuffer,
					SCRIPTLET_INIDOTSIZE, sp->s_dotbufini,
					sizeof(*sp->s_dotbuffer));
	if (nrequested != newsize)
	{
	    sp->s_dotoffset = 0;
	    sp->s_dotbuffer[0] = 0;
	    return (0);
	}
    }
    strcpy(sp->s_dotbuffer + sp->s_dotoffset, st);
    sp->s_dotoffset += len;
    return (1);
}

static int scriptlet_dotfloat(t_scriptlet *sp, float f)
{
    char obuf[32];
    sprintf(obuf, "%g", f);
    return (scriptlet_dotstring(sp, obuf));
}

static char *scriptlet_dedot(t_scriptlet *sp, char *ibuf,
			     int resolveall, int visedonly,
			     int ac, t_atom *av, t_props *argprops)
{
    int len = 0;
    char *obuf = sp->s_dotbuffer;
    sp->s_dotoffset = 0;
    switch (*ibuf)
    {
    case '#':
	if (resolveall)
	{
	    int which = ibuf[1] - '1';
	    if (which >= 0 && which < 9)
	    {
		if (which < ac)
		{
		    av += which;
		    if (av->a_type == A_FLOAT)
			sprintf(obuf, "%g", av->a_w.w_float);
		    else if (av->a_type == A_SYMBOL && av->a_w.w_symbol)
			scriptlet_dotstring(sp, av->a_w.w_symbol->s_name);
		    else
			obuf[0] = 0;  /* LATER rethink */
		}
		else strcpy(obuf, "0");
		len = 2;
	    }
	    else if (!strncmp(&ibuf[1], "args", 4))
	    {
		if (ac) while (1)
		{
		    if (av->a_type == A_FLOAT)
			scriptlet_dotfloat(sp, av->a_w.w_float);
		    else if (av->a_type == A_SYMBOL && av->a_w.w_symbol)
			scriptlet_dotstring(sp, av->a_w.w_symbol->s_name);
		    else
		    {  /* LATER rethink */
			obuf[0] = 0;
			break;
		    }
		    ac--; av++;
		    if (ac)
			sp->s_dotbuffer[sp->s_dotoffset++] = ' ';
		    else
			break;
		}
		else obuf[0] = 0;
		len = 5;
	    }
	    else if (argprops)
	    {
		char *iptr, *optr, c;
		int cnt;
		for (iptr = ibuf + 1, c = *iptr, cnt = 1; c;
		     iptr++, c = *iptr, cnt++)
		{
		    if ((c < 'A' || c > 'Z') && (c < 'a' || c > 'z'))
		    {
			*iptr = 0;
			break;
		    }
		}
		if (optr = props_getvalue(argprops, ibuf + 1))
		{
		    scriptlet_dotstring(sp, optr);
		    len = cnt;
		}
		if (c) *iptr = c;
	    }
	}
	break;
    case '-':
	if (resolveall && sp->s_item)
	{
	    t_canvas *cv;
	    if (cv = scriptlet_canvasvalidate(sp, visedonly))
	    {
		sprintf(obuf, ".x%lx.c.%s%x", (int)cv, sp->s_item->s_name,
			(int)sp->s_owner);
		len = 1;
	    }
	}
	break;
    case '^':
	if (resolveall)
	{
	    t_canvas *cv;
	    if (cv = scriptlet_canvasvalidate(sp, visedonly))
	    {
		sprintf(obuf, ".x%lx", (int)cv);
		len = 1;
	    }
	}
	break;
    case '|':
	if (resolveall)
	{
	    strcpy(obuf, sp->s_cbtarget->s_name);
	    len = 1;
	}
	break;
    case '~':  /* FIXME, the dot-tilde stuff is purely experimental. */
	if (resolveall)
	{
	    t_canvas *cv;
	    if (cv = scriptlet_canvasvalidate(sp, 0))
	    {
		t_glist *glist;
		if (!strncmp(&ibuf[1], "tag", 3))
		{
		    t_rtext *rt;
		    glist = cv->gl_owner;
		    if (glist && glist_isvisible(glist) && glist->gl_editor
			&& (rt = glist_findrtext(glist, (t_object *)cv)))
			sprintf(obuf, "%s", rtext_gettag(rt));
		    else
			obuf[0] = 0;
		    len = 4;
		}
		else if (!strncmp(&ibuf[1], "parent", 6))
		{
		    glist = cv->gl_owner;
		    if (glist && glist_isvisible(glist))
			sprintf(obuf, ".x%lx", (int)glist);
		    else
			obuf[0] = 0;
		    len = 7;
		}
		else if (!strncmp(&ibuf[1], "root", 4))
		{
		    glist = canvas_getrootfor(cv);
		    if (glist && glist_isvisible(glist))
			sprintf(obuf, ".x%lx", (int)glist);
		    else
			obuf[0] = 0;
		    len = 5;
		}
		else if (!strncmp(&ibuf[1], "owner", 5))
		{
		    if (glist = canvas_getrootfor(cv))
			glist = glist->gl_owner;
		    if (glist && glist_isvisible(glist))
			sprintf(obuf, ".x%lx", (int)glist);
		    else
			obuf[0] = 0;
		    len = 6;
		}
		else if (!strncmp(&ibuf[1], "top", 3))
		{
		    glist = cv;
		    while (glist->gl_owner) glist = glist->gl_owner;
		    if (glist && glist_isvisible(glist))
			sprintf(obuf, ".x%lx", (int)glist);
		    else
			obuf[0] = 0;
		    len = 4;
		}
		/* LATER find out when gl_<coords> are updated,
		   think how to better sync them to Tk. */
		else if (!strncmp(&ibuf[1], "x1", 2))
		{
		    sprintf(obuf, "%d", cv->gl_screenx1);
		    len = 3;
		}
		else if (!strncmp(&ibuf[1], "x2", 2))
		{
		    sprintf(obuf, "%d", cv->gl_screenx2);
		    len = 3;
		}
		else if (!strncmp(&ibuf[1], "y1", 2))
		{
		    sprintf(obuf, "%d", cv->gl_screeny1);
		    len = 3;
		}
		else if (!strncmp(&ibuf[1], "y2", 2))
		{
		    sprintf(obuf, "%d", cv->gl_screeny2);
		    len = 3;
		}
		else if (!strncmp(&ibuf[1], "edit", 4))
		{
		    sprintf(obuf, "%d", cv->gl_edit);
		    len = 5;
		}
		else if (!strncmp(&ibuf[1], "gop", 3))
		{
		    sprintf(obuf, "%d", glist_isgraph(cv));
		    len = 4;
		}
		else if (!strncmp(&ibuf[1], "dir", 3))
		{
		    sprintf(obuf, "%s", canvas_getdir(cv)->s_name);
		    len = 4;
		}
		else loud_error(sp->s_owner, "bad field '%s'", &ibuf[1]);
	    }
	}
	break;
    case '`':
	sprintf(obuf, "\\");
	len = 1;
	break;
    case ':':
	sprintf(obuf, ";");
	len = 1;
	break;
    case '(':
	sprintf(obuf, "{");
	len = 1;
	break;
    case ')':
	sprintf(obuf, "}");
	len = 1;
	break;
    case '<':
	if (resolveall)
	{
	    if (ibuf[1] == ':')
	    {
		sprintf(obuf, "{pdsend [concat ");
		len = 2;
	    }
	    else if (ibuf[1] == '|')
	    {
		sprintf(obuf, "{pdsend [concat %s ", sp->s_rptarget->s_name);
		len = 2;
	    }
	    else
	    {
		sprintf(obuf, "{pdsend [concat %s _cb ", sp->s_cbtarget->s_name);
		len = 1;
	    }
	}
	break;
    case '>':
	if (resolveall)
	{
	    sprintf(obuf, "\\;]}");
	    len = 1;
	}
	break;
    }
    return (len ? ibuf + len : 0);
}

int scriptlet_isempty(t_scriptlet *sp)
{
    return (!(sp->s_tail > sp->s_head && *sp->s_head));
}

void scriptlet_reset(t_scriptlet *sp)
{
    sp->s_cvstate = SCRIPTLET_CVUNKNOWN;
    sp->s_locked = 0;
    sp->s_separator = 0;
    strcpy(sp->s_buffer, "namespace eval ::toxy {\
 proc query {} {set ::toxy::reply [\n");
    sp->s_head = sp->s_tail = sp->s_buffer + strlen(sp->s_buffer);
}

void scriptlet_prealloc(t_scriptlet *sp, int sz, int mayshrink)
{
    if (sz < SCRIPTLET_INISIZE)
	sz = SCRIPTLET_INISIZE;
    if (sz < sp->s_size && mayshrink)
    {
	if (sp->s_buffer != sp->s_bufini)
	    freebytes(sp->s_buffer, sp->s_size * sizeof(*sp->s_buffer));
	else
	    loudbug_bug("scriptlet_prealloc");
	sp->s_size = SCRIPTLET_INISIZE;
	sp->s_buffer = sp->s_bufini;
    }
    if (sz > sp->s_size)
	sp->s_buffer = grow_nodata(&sz, &sp->s_size, sp->s_buffer,
				   SCRIPTLET_INISIZE, sp->s_bufini,
				   sizeof(*sp->s_buffer));
    scriptlet_reset(sp);
}

static int scriptlet_addstring(t_scriptlet *sp, char *ibuf,
			       int resolveall, int visedonly,
			       int ac, t_atom *av, t_props *argprops)
{
    int result = 1;
    char *bp = ibuf, *ep = ibuf, *ep1;
    if (!sp->s_separator)
	sp->s_separator = ' ';
    while (*ep)
    {
	if (*ep == '.'
	    && (ep1 = scriptlet_dedot(sp, ep + 1, resolveall, visedonly,
				      ac, av, argprops)))
	{
	    *ep = 0;
	    if (!(result = scriptlet_doappend(sp, bp)))
		break;
	    *ep = '.';
	    if (!(result = scriptlet_doappend(sp, sp->s_dotbuffer)))
		break;
	    bp = ep = ep1;
	}
	else ep++;
    }
    if (result)
	result = scriptlet_doappend(sp, bp);
    sp->s_separator = 0;
    return (result);
}

static int scriptlet_addfloat(t_scriptlet *sp, t_float f)
{
    char buf[64];
    if (!sp->s_separator)
	sp->s_separator = ' ';
    sprintf(buf, "%g", f);
    return (scriptlet_doappend(sp, buf));
}

int scriptlet_add(t_scriptlet *sp,
		  int resolveall, int visedonly, int ac, t_atom *av)
{
    while (ac--)
    {
	int result = 1;
	if (av->a_type == A_SYMBOL)
	    result = scriptlet_addstring(sp, av->a_w.w_symbol->s_name,
					 resolveall, visedonly, 0, 0, 0);
	else if (av->a_type == A_FLOAT)
	    result = scriptlet_addfloat(sp, av->a_w.w_float);
	if (!result)
	    return (0);
	av++;
    }
    return (1);
}

void scriptlet_setseparator(t_scriptlet *sp, char c)
{
    sp->s_separator = c;
}

void scriptlet_push(t_scriptlet *sp)
{
    if (scriptlet_ready(sp))
    {
	char *tail = sp->s_tail;
	strcpy(tail, "\n");
	sys_gui(sp->s_head);
	*tail = 0;
    }
}

void scriptlet_qpush(t_scriptlet *sp)
{
    if (scriptlet_ready(sp))
    {
	char buf[MAXPDSTRING];
	char *tail = sp->s_tail;
	/* Could not find any other way, than to postpone processing of the
	   query, after everything which might have been glued to our tail,
	   is evaluated.  Otherwise, any command arriving later, during
	   blocking of the query (e.g. in a Tk dialog), would be evaluated
	   prior to our tail, via Tcl_DoOneEvent().  We postpone also the
	   setup step (defining the query proc), in order to be able to
	   handle several queries at once.  All this is far from ideal --
	   the sequence "query this, tot that", is swapped, unless written
	   as "query this, tot after 0 .(that.)", which is going to cause
	   much confusion...  LATER revisit.  Do not forget, that since
	   pd_readsocket() is not reentrant, sys_gui()d commands should never
	   enter event loop directly by blocking on a dialog, vwait, etc.,
	   because the pd_readsocket handler is event-driven on unix. */
	sys_gui("after 0 {\n");
	strcpy(tail, "]}}\n");
	sys_gui(sp->s_buffer);
	*tail = 0;
	sprintf(buf, "\
 if {[info tclversion] < 8.4} {\n\
  trace variable ::toxy::reply w \"::toxy::doreply %s\"\n\
 } else {\n\
  trace add variable ::toxy::reply write \"::toxy::doreply %s\"\n\
 }\n\
 ::toxy::query}\n", sp->s_rptarget->s_name, sp->s_rptarget->s_name);
	sys_gui(buf);
    }
}

/* Non-substituting -- LATER think if this is likely to cause any confusion.
   Especially, consider the widget_vis() vs. widget_update() case. */
void scriptlet_vpush(t_scriptlet *sp, char *varname)
{
    if (scriptlet_ready(sp))
    {
	char *tail = sp->s_tail;
	strcpy(tail, "}\n");
	sys_vgui("set ::toxy::%s { ", varname);
	sys_gui(sp->s_head);
	*tail = 0;
    }
}

int scriptlet_evaluate(t_scriptlet *insp, t_scriptlet *outsp, int visedonly,
		       int ac, t_atom *av, t_props *argprops)
{
    if (scriptlet_ready(insp))
    {
	t_atom *ap;
	int i;
	char *bp;
	char separator = 0;
	/* FIXME pregrowing of the transient scriptlet */
	scriptlet_reset(outsp);
	/* LATER abstract this into scriptlet_parse() */
	bp = insp->s_head;
	while (*bp)
	{
	    if (*bp == '\n')
		separator = '\n';
	    else if (*bp == ' ' || *bp == '\t')
	    {
		if (!separator) separator = ' ';
	    }
	    else
	    {
		int done = 1;
		char *ep = bp;
		char c = ' ';
		while (*++ep)
		{
		    if (*ep == ' ' || *bp == '\t' || *ep == '\n')
		    {
			done = 0;
			c = *ep;
			*ep = 0;
			break;
		    }
		}
		outsp->s_separator = separator;
		scriptlet_addstring(outsp, bp, 1, visedonly, ac, av, argprops);
		if (done)
		    break;
		*ep = c;
		bp = ep;
		separator = (c == '\t' ? ' ' : c);
	    }
	    bp++;
	}
	return (outsp->s_cvstate != SCRIPTLET_CVMISSING);
    }
    else return (0);
}

/* utility function to be used in a comment-parsing callback */
char *scriptlet_nextword(char *buf)
{
    while (*++buf)
    {
	if (*buf == ' ' || *buf == '\t')
	{
	    char *ptr = buf + 1;
	    while (*ptr == ' ' || *ptr == '\t') ptr++;
	    *buf = 0;
	    return (*ptr ? ptr : 0);
	}
    }
    return (0);
}

static t_verslet *verslet_new(t_pd *owner)
{
    t_verslet *vp = getbytes(sizeof(*vp));
    vp->v_owner = owner;
    vp->v_package[0] = 0;
    vp->v_version[0] = 0;
    return (vp);
}

static void verslet_free(t_verslet *vp)
{
    freebytes(vp, sizeof(*vp));
}

static void verslet_set(t_verslet *vp, char *pname, char *vname)
{
    strncpy(vp->v_package, pname, VERSLET_MAXPACKAGE-1);
    vp->v_package[VERSLET_MAXPACKAGE-1] = 0;
    strncpy(vp->v_version, vname, VERSLET_MAXVERSION-1);
    vp->v_version[VERSLET_MAXVERSION-1] = 0;
}

static int verslet_parse(t_verslet *vp, char *buf, int multiline)
{
    char *ptr = buf;
    int plen = 0;
    vp->v_package[0] = 0;
    vp->v_version[0] = 0;
    if (multiline)
    {
	while (*ptr)
	{
	    while (*ptr == ' ' || *ptr == '\t') ptr++;
	    if (strncmp(ptr, "package", 7))
	    {
		while (*ptr && *ptr != '\n') ptr++;
		if (*ptr)
		    buf = ++ptr;
	    }
	    else break;
	}
	if (*ptr)
	    ptr += 7;
	else
	    ptr = 0;
    }
    else
    {
	while (*ptr == ' ' || *ptr == '\t') ptr++;
	if (strncmp(ptr, "package", 7))
	    ptr = 0;
	else
	    ptr += 7;
    }
    if (ptr)
    {
	while (*ptr == ' ' || *ptr == '\t') ptr++;
	if (!strncmp(ptr, "provide", 7))
	{
	    ptr += 7;
	    while (*ptr == ' ' || *ptr == '\t') ptr++;
	    if (*ptr)
	    {
		for (plen = 0; plen < VERSLET_MAXPACKAGE-1 && *ptr;
		     plen++, ptr++)
		{
		    if (*ptr == '\n' || *ptr == '\r')
			break;
		    else if (*ptr == ' ' || *ptr == '\t')
		    {
			vp->v_package[plen] = 0;
#ifdef SCRIPTLET_DEBUG
			loudbug_post("package \"%s\"", vp->v_package);
#endif
			while (*ptr == ' ' || *ptr == '\t') ptr++;
			if (*ptr >= '0' && *ptr <= '9')
			{
			    int vlen;
			    for (vlen = 0; vlen < VERSLET_MAXVERSION-1 && *ptr;
				 vlen++, ptr++)
			    {
				if ((*ptr >= '0' && *ptr <= '9') || *ptr == '.')
				    vp->v_version[vlen] = *ptr;
				else
				    break;
			    }
			    if (vlen)
			    {
				vp->v_version[vlen] = 0;
#ifdef SCRIPTLET_DEBUG
				loudbug_post("version \"%s\"", vp->v_version);
#endif
				return (1);
			    }
			}
			break;
		    }
		    else vp->v_package[plen] = *ptr;
		}
	    }
	}
	if (plen)
	    loud_error(vp->v_owner,
		       "incomplete scriptlet version declaration \"%s\"", buf);
    }
    return (0);
}

static int verslet_compare(t_verslet *vp1, t_verslet *vp2)
{
    char *vname1 = vp1->v_version, *vname2 = vp2->v_version;
    while (*vname1 || *vname2)
    {
	int v1, v2;
	for (v1 = 0; *vname1 >= '0' && *vname1 <= '9'; vname1++)
	    v1 = v1 * 10 + *vname1 - '0';
	for (v2 = 0; *vname2 >= '0' && *vname2 <= '9'; vname2++)
	    v2 = v2 * 10 + *vname2 - '0';
	if (v1 < v2)
	    return (-1);
	else if (v1 > v2)
	    return (1);
	if (*vname1)
	{
	    if (*vname1 == '.')
		*vname1++;
	    if (*vname1 < '0' || *vname1 > '9')
	    {
		loud_error(vp1->v_owner, "invalid version \"%s\"",
			   vp1->v_version);
		while (*vname1) *vname1++;
	    }
	}
	if (*vname2)
	{
	    if (*vname2 == '.')
		*vname2++;
	    if (*vname2 < '0' || *vname2 > '9')
	    {
		loud_error(vp2->v_owner, "invalid version \"%s\"",
			   vp2->v_version);
		while (*vname2) *vname2++;
	    }
	}
    }
    return (0);
}

static int scriptlet_doread(t_scriptlet *sp, t_pd *caller, FILE *fp,
			    char *rc, t_verslet *vcompare,
			    char *builtin, t_scriptlet_cmntfn cmntfn)
{
    t_scriptlet *outsp = sp, *newsp;
    t_verslet *vp;
    int vdiff = 0;
    char buf[MAXPDSTRING];
    if (!caller) caller = sp->s_owner;
    vp = (vcompare ? verslet_new(caller) : 0);
    while ((fp && !feof(fp) && fgets(buf, MAXPDSTRING - 1, fp))
	   || builtin)
    {
	char *ptr;
	if (builtin)
	{
	    int i;
	    for (i = 0, ptr = buf; i < MAXPDSTRING - 1; i++, ptr++)
	    {
		if ((*ptr = (*builtin ? *builtin : '\n')) == '\n')
		{
		    ptr[1] = 0;
		    if (*builtin) builtin++;
		    if (!*builtin) builtin = 0;
		    break;
		}
		else builtin++;
	    }
	}
	else
	{
	    for (ptr = buf; *ptr; ptr++)
		if (*ptr == '\r')
		    *ptr = ' ';  /* LATER rethink */
	    if (vp && verslet_parse(vp, buf, 0))
	    {
		if (vdiff = verslet_compare(vp, vcompare))
		    goto readfailed;
		else
		{
		    verslet_free(vp);
		    vp = 0;
		}
	    }
	}
	ptr = buf;
	while (*ptr == ' ' || *ptr == '\t') ptr++;
	if (*ptr == '#')
	{
	    if (cmntfn)
	    {
		char sel = *++ptr;
		if (sel && sel != '\n')
		{
		    ptr++;
		    while (*ptr == ' ' || *ptr == '\t') ptr++;
		    if (*ptr == '\n')
			*ptr = 0;
		    if (*ptr)
		    {
			char *ep = ptr + strlen(ptr) - 1;
			while (*ep == ' ' || *ep == '\t' || *ep == '\n')
			    ep--;
			ep[1] = 0;
		    }
		    if (vp)
			goto readfailed;  /* FIXME call a request cmntfn? */
		    newsp = cmntfn(caller, rc, sel, ptr);
		    if (newsp == SCRIPTLET_UNLOCK)
			outsp->s_locked = 0;
		    else if (newsp == SCRIPTLET_LOCK)
			outsp->s_locked = 1;
		    else if (newsp != outsp)
		    {
			outsp->s_locked = 0;
			outsp = newsp;
		    }
		}
	    }
	}
	else if (*ptr && *ptr != '\n')
	    scriptlet_doappend(outsp, buf);
    }
readfailed:
    outsp->s_locked = 0;
    if (vp)
    {
	verslet_free(vp);
	scriptlet_reset(sp);
	if (vdiff < 0)
	    return (SCRIPTLET_OLDERVERSION);
	else if (vdiff > 0)
	    return (SCRIPTLET_NEWERVERSION);
	else
	    return (SCRIPTLET_NOVERSION);
    }
    else return (SCRIPTLET_OK);
}

/* Load particular section(s) from buffer (skip up to an unlocking comment,
   keep appending up to a locking comment, repeat). */
int scriptlet_rcparse(t_scriptlet *sp, t_pd *caller, char *rc, char *contents,
		      t_scriptlet_cmntfn cmntfn)
{
    int result;
    sp->s_locked = 1;  /* see scriptlet_doread() above for unlocking scheme */
    result = scriptlet_doread(sp, caller, 0, rc, 0, contents, cmntfn);
    return (result);
}

int scriptlet_rcload(t_scriptlet *sp, t_pd *caller, char *rc, char *ext,
		     char *builtin, t_scriptlet_cmntfn cmntfn)
{
    int result;
    char filename[MAXPDSTRING], buf[MAXPDSTRING], *nameptr, *dir;
    int fd;
    if (sp->s_glist)
	dir = canvas_getdir(sp->s_glist)->s_name;
    else
	dir = "";  /* which means pwd, usually the same as at Pd startup... */
    if ((fd = open_via_path(dir, rc, ext, buf, &nameptr, MAXPDSTRING, 0)) < 0)
    {
	result = SCRIPTLET_NOFILE;
    }
    else
    {
	FILE *fp;
    	close(fd);
	if (nameptr != buf)
	{
	    strcpy(filename, buf);
	    strcat(filename, "/");
	    strcat(filename, nameptr);
	    sys_bashfilename(filename, filename);
	}
	else sys_bashfilename(nameptr, filename);
	if (fp = fopen(filename, "r"))
	{
	    t_verslet *vp;
	    if (builtin)
	    {
		vp = verslet_new(sp->s_owner);
		if (!verslet_parse(vp, builtin, 1))
		{
		    loudbug_bug("scriptlet_rcload 1");
		    verslet_free(vp);
		    vp = 0;
		}
	    }
	    else vp = 0;
	    result = scriptlet_doread(sp, caller, fp, rc, vp, 0, cmntfn);
	    fclose(fp);
	    if (vp)
		verslet_free(vp);
	}
	else
	{
	    loudbug_bug("scriptlet_rcload 2");
	    result = SCRIPTLET_NOFILE;
	}
    }
    if (result != SCRIPTLET_OK)
    {
	scriptlet_doread(sp, caller, 0, rc, 0, builtin, cmntfn);
    }
    return (result);
}

int scriptlet_read(t_scriptlet *sp, t_symbol *fn)
{
    int result;
    FILE *fp;
    char buf[MAXPDSTRING];
    post("loading scriptlet file \"%s\"", fn->s_name);
    /* FIXME use open_via_path() */
    if (sp->s_glist)
	canvas_makefilename(sp->s_glist, fn->s_name, buf, MAXPDSTRING);
    else
	strncpy(buf, fn->s_name, MAXPDSTRING);
    sys_bashfilename(buf, buf);
    if (fp = fopen(buf, "r"))
    {
	scriptlet_reset(sp);
	result = scriptlet_doread(sp, 0, fp, 0, 0, 0, 0);
	fclose(fp);
    }
    else
    {
	loud_error(sp->s_owner, "error while loading file \"%s\"", fn->s_name);
    	result = SCRIPTLET_NOFILE;
    }
    return (result);
}

int scriptlet_write(t_scriptlet *sp, t_symbol *fn)
{
    int size = sp->s_tail - sp->s_head;
    if (size > 0 && *sp->s_head)
    {
	FILE *fp;
	char buf[MAXPDSTRING];
	post("saving scriptlet file \"%s\"", fn->s_name);
	if (sp->s_glist)
	    canvas_makefilename(sp->s_glist, fn->s_name, buf, MAXPDSTRING);
	else
	    strncpy(buf, fn->s_name, MAXPDSTRING);
	sys_bashfilename(buf, buf);
	if (fp = fopen(buf, "w"))
	{
	    int result = fwrite(sp->s_head, 1, size, fp);
	    fclose(fp);
	    if (result == size)
		return (SCRIPTLET_OK);
	}
	loud_error(sp->s_owner, "error while saving file \"%s\"", fn->s_name);
	return (fp ? SCRIPTLET_BADFILE : SCRIPTLET_NOFILE);
    }
    else
    {
	loud_warning(sp->s_owner, "scriptlet", "empty scriptlet not written");
	return (SCRIPTLET_IGNORED);
    }
}

char *scriptlet_getcontents(t_scriptlet *sp, int *lenp)
{
    *lenp = sp->s_tail - sp->s_head;
    return (sp->s_head);
}

char *scriptlet_getbuffer(t_scriptlet *sp, int *sizep)
{
    *sizep = sp->s_size;
    return (sp->s_buffer);
}

void scriptlet_setowner(t_scriptlet *sp, t_pd *owner)
{
    sp->s_owner = owner;
}

void scriptlet_clone(t_scriptlet *to, t_scriptlet *from)
{
    scriptlet_reset(to);
    to->s_separator = ' ';
    /* LATER add a flag to optionally use from's buffer with refcount */
    scriptlet_doappend(to, from->s_head);
}

void scriptlet_append(t_scriptlet *to, t_scriptlet *from)
{
    to->s_separator = ' ';
    scriptlet_doappend(to, from->s_head);
}

void scriptlet_free(t_scriptlet *sp)
{
    if (sp)
    {
	if (sp->s_buffer != sp->s_bufini)
	    freebytes(sp->s_buffer, sp->s_size * sizeof(*sp->s_buffer));
	if (sp->s_dotbuffer != sp->s_dotbufini)
	    freebytes(sp->s_dotbuffer,
		      sp->s_dotsize * sizeof(*sp->s_dotbuffer));
	freebytes(sp, sizeof(*sp));
    }
}

/* The parameter 'gl' (null accepted) is necessary, because the 's_glist'
   field, if implicitly set, would be dangerous (after a glist is gone)
   and confusing (current directory used for i/o of a global scriptlet). */
t_scriptlet *scriptlet_new(t_pd *owner,
			   t_symbol *rptarget, t_symbol *cbtarget,
			   t_symbol *item, t_glist *glist,
			   t_scriptlet_cvfn cvfn)
{
    t_scriptlet *sp = getbytes(sizeof(*sp));
    if (sp)
    {
	static int configured = 0;
	if (!configured)
	{
	    sys_gui("image create bitmap ::toxy::img::empty -data {}\n");
	    sys_gui("proc ::toxy::doreply {target vname vndx op} {\n");
	    sys_gui(" pd [concat $target _rp $::toxy::reply \\;]\n");
	    sys_gui(" unset ::toxy::reply\n");
	    sys_gui("}\n");
        /* if older than 0.43, create an 0.43-style pdsend */
        sys_gui("if {[llength [info procs ::pdsend]] == 0} {");
        sys_gui("proc ::pdsend {args} {::pd \"[join $args { }] ;\"}}\n");
	    configured = 1;
	}
	sp->s_owner = owner;
	sp->s_glist = glist;
	sp->s_rptarget = rptarget;
	sp->s_cbtarget = cbtarget;
	sp->s_item = item;
	sp->s_cvfn = cvfn;
	sp->s_size = SCRIPTLET_INISIZE;
	sp->s_buffer = sp->s_bufini;
	sp->s_dotsize = SCRIPTLET_INIDOTSIZE;
	sp->s_dotoffset = 0;
	sp->s_dotbuffer = sp->s_dotbufini;
	scriptlet_reset(sp);
    }
    return (sp);
}

t_scriptlet *scriptlet_newalike(t_scriptlet *sp)
{
    return (scriptlet_new(sp->s_owner, sp->s_rptarget, sp->s_cbtarget,
			  sp->s_item, sp->s_glist, sp->s_cvfn));
}
