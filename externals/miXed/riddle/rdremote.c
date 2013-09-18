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

EXTERN_STRUCT _rdpool;
#define t_rdpool  struct _rdpool

typedef struct _rdenvironment
{
    t_pd         en_pd;
    t_rdpool    *en_graphpools;
    t_rdremote  *en_writers;
    t_rdremote  *en_readers;  /* list of orphaned readers */
    t_clock     *en_updatedspclock;
} t_rdenvironment;

static t_class *rdenvironment_class = 0;

#define RDREMOTE_INISIZE  1024

struct _rdremote
{
    int          re_id;        /* positive for readers, otherwise zero */
    t_symbol    *re_name;
    t_riddle    *re_owner;
    int          re_phase;
    t_rdremote  *re_portlink;  /* slist of object i/o */
    t_rdremote  *re_prev;      /* double-linked list of readers or writers */
    t_rdremote  *re_next;

    /* common part, copied from writer to all its readers
       immediately after any change */
    int          re_nframes;
    int          re_framesize;
    int          re_npoints;
    int          re_maxphase;
    t_float     *re_data;

    /* writer-specific part */
    t_rdremote  *re_readers;
    int          re_datasize;
    t_float     *re_inidata;
};

#define RDPICKER_INISIZE  64

struct _rdpool
{
    t_canvas    *po_graph;
    int          po_refcount;
    t_rdpicker  *po_pickstore;
    t_rdpool    *po_prev;
    t_rdpool    *po_next;
};

struct _rdpicker
{
    t_symbol    *pi_key;
    int          pi_refcount;
    t_rdpool    *pi_graphpool;
    int          pi_size;
    int          pi_maxsize;
    t_float     *pi_data;
    t_float      pi_inidata[RDPICKER_INISIZE];
    t_rdpicker  *pi_prev;
    t_rdpicker  *pi_next;
};

typedef struct _rdfeeder
{
    t_rdpicker        *fe_picker;
    struct _rdfeeder  *fe_next;
} t_rdfeeder;

struct _rdfeedchain
{
    t_rdfeeder  *ch_head;
    int          ch_outno;
};

static t_symbol *rdps__idle = 0;

static void rdenvironment_updatedsptick(t_rdenvironment *en)
{
    canvas_update_dsp();
}

static void rdenvironment_anything(t_rdenvironment *en,
				   t_symbol *s, int ac, t_atom *av)
{
    /* FIXME */
}

static t_rdenvironment *rdenvironment_provide(void)
{
    t_rdenvironment *en;
    t_symbol *ps__riddle = gensym("_riddle");     /* class name */
    t_symbol *ps_hashriddle = gensym("#riddle");  /* instance binding */
    if (ps_hashriddle->s_thing)
    {
	char *cname = class_getname(*ps_hashriddle->s_thing);
	if (strcmp(cname, ps__riddle->s_name))
	{
	    /* FIXME protect against the danger of someone else
	       (e.g. receive) binding to #riddle */
	    loudbug_bug("rdenvironment_provide");
	}
	else
	{
	    /* FIXME compatibility test */
	    rdenvironment_class = *ps_hashriddle->s_thing;
	    return ((t_rdenvironment *)ps_hashriddle->s_thing);
	}
    }
    rdps__idle = gensym("_idle");
    rdenvironment_class = class_new(ps__riddle, 0, 0,
				    sizeof(t_rdenvironment),
				    CLASS_PD | CLASS_NOINLET, 0);
    class_addanything(rdenvironment_class, rdenvironment_anything);
    en = (t_rdenvironment *)pd_new(rdenvironment_class);  /* never freed */
    en->en_graphpools = 0;
    en->en_writers = 0;
    en->en_readers = 0;
    en->en_updatedspclock =
	clock_new(en, (t_method)rdenvironment_updatedsptick);
    pd_bind((t_pd *)en, ps_hashriddle);  /* never unbound */
    return (en);
}

t_pd *riddle_getenvironment(void)
{
    return ((t_pd *)rdenvironment_provide());
}


/* subgraph-localized remote connections: feeders and pickers
   LATER should become a kind of rdremote writers and readers */

t_rdpicker *rdpicker_attach(t_riddle *rd, t_symbol *key)
{
    t_rdenvironment *en = rdenvironment_provide();
    /* FIXME protect against calling outside of the constructor */
    t_canvas *graph = canvas_getcurrent();
    t_rdpool *po;
    t_rdpicker *pi;
    for (po = en->en_graphpools; po; po = po->po_next)
    {
	if (po->po_graph == graph)
	{
	    for (pi = po->po_pickstore; pi; pi = pi->pi_next)
	    {
		if (pi->pi_key == key)
		{
		    pi->pi_refcount++;
		    return (pi);
		}
	    }
	}
    }

    if (po)
	po->po_refcount++;
    else
    {
	po = getbytes(sizeof(*po));
	po->po_graph = graph;
	po->po_refcount = 1;
	po->po_pickstore = 0;
	po->po_prev = 0;
	if (en->en_graphpools)
	    en->en_graphpools->po_prev = po;
	po->po_next = en->en_graphpools;
	en->en_graphpools = po;
    }

    pi = getbytes(sizeof(*pi));
    pi->pi_key = key;
    pi->pi_refcount = 1;
    pi->pi_graphpool = po;
    pi->pi_size = 0;
    pi->pi_maxsize = RDPICKER_INISIZE;
    pi->pi_data = pi->pi_inidata;
    pi->pi_prev = 0;
    if (po->po_pickstore)
	po->po_pickstore->pi_prev = pi;
    pi->pi_next = po->po_pickstore;
    po->po_pickstore = pi;
    return (pi);
}

void rdpicker_detach(t_rdpicker *pi, t_riddle *rd)
{
    if (pi->pi_refcount > 1)
	pi->pi_refcount--;
    else
    {
	if (pi->pi_data && pi->pi_data != pi->pi_inidata)
	    freebytes(pi->pi_data, pi->pi_maxsize * sizeof(*pi->pi_data));
	if (pi->pi_prev)
	    pi->pi_prev->pi_next = pi->pi_next;
	else if (pi->pi_graphpool)
	    pi->pi_graphpool->po_pickstore = pi->pi_next;
	else
	    loudbug_bug("rdpicker_detach 1");
	if (pi->pi_next)
	    pi->pi_next->pi_prev = pi->pi_prev;
	if (pi->pi_graphpool)
	{
	    t_rdpool *po = pi->pi_graphpool;
	    t_rdenvironment *en = rdenvironment_provide();
	    if (po->po_refcount > 1)
		po->po_refcount--;
	    else
	    {
		if (po->po_pickstore)
		    loudbug_bug("rdpicker_detach 2");
		if (po->po_prev)
		    po->po_prev->po_next = po->po_next;
		else
		    en->en_graphpools = po->po_next;
		if (po->po_next)
		    po->po_next->po_prev = po->po_prev;
		freebytes(po, sizeof(*po));
	    }
	}
	freebytes(pi, sizeof(*pi));
    }
}

t_float *rdpicker_pick(t_rdpicker *pi, int *sizep)
{
    *sizep = pi->pi_size;
    return (pi->pi_data);
}

t_float rdpicker_pick1(t_rdpicker *pi)
{
    return (*pi->pi_data);
}

static t_rdpicker *rdpool_linkpicker(t_rdpool *po, t_symbol *key, int size)
{
    t_rdpicker *pi;
    for (pi = po->po_pickstore; pi; pi = pi->pi_next)
    {
	if (pi->pi_key == key)
	{
	    if (size > pi->pi_maxsize)
		pi->pi_data = grow_nodata(&size, &pi->pi_maxsize, pi->pi_data,
					  RDPICKER_INISIZE, pi->pi_inidata,
					  sizeof(*pi->pi_data));
	    pi->pi_size = size;
	    return (pi);
	}
    }
    return (0);
}

/* LATER think about rdpool_unlinkpicker() */

static void rdfeedchain_proliferate(t_rdfeedchain *ch, t_rdpool *pohead,
				    t_gobj *g, t_symbol *key, int size)
{
    int result = 0;
    for (; g; g = g->g_next)
    {
        if (pd_class((t_pd *)g) == canvas_class)
	{
	    t_canvas *graph = (t_canvas *)g;
	    t_rdpool *po;
	    t_rdpicker *pi;
	    for (po = pohead; po; po = po->po_next)
		if (po->po_graph == graph)
		    break;
	    if (po && (pi = rdpool_linkpicker(po, key, size)))
	    {
		t_rdfeeder *fe = getbytes(sizeof(*fe));  /* FIXME reuse */
		fe->fe_picker = pi;
		fe->fe_next = ch->ch_head;
		ch->ch_head = fe;
	    }
            rdfeedchain_proliferate(ch, pohead, graph->gl_list, key, size);
	}
    }
}

void rdfeedchain_free(t_rdfeedchain *ch)
{
    t_rdfeeder *fe, *fenext;
    for (fe = ch->ch_head; fe; fe = fenext)
    {
	fenext = fe->fe_next;
	freebytes(fe, sizeof(*fe));
    }
}

t_rdfeedchain *rdfeedchain_new(int outno)
{
    t_rdfeedchain *ch = getbytes(sizeof(*ch));
    ch->ch_head = 0;
    ch->ch_outno = outno;
}

t_rdfeedchain *riddle_usefeedchain(t_riddle *rd,
				   int sigoutno, t_symbol *key, int size)
{
    t_rdfeedchain *ch;
    if (ch = riddle_getfeedchain(rd, sigoutno))
    {
	t_canvas *graph;
	t_rdfeeder *fe, *fenext;
	/* LATER reuse */
	for (fe = ch->ch_head; fe; fe = fenext)
	{
	    fenext = fe->fe_next;
	    freebytes(fe, sizeof(*fe));
	}
	for (graph = riddle_firstgraph(rd, ch->ch_outno);
	     graph; graph = riddle_nextgraph(rd))
	{
	    t_rdenvironment *en = rdenvironment_provide();
	    t_rdpool *po;
	    t_rdpicker *pi;
	    for (po = en->en_graphpools; po; po = po->po_next)
		if (po->po_graph == graph)
		    break;
	    if (po && (pi = rdpool_linkpicker(po, key, size)))
	    {
		t_rdfeeder *fe = getbytes(sizeof(*fe));
		fe->fe_picker = pi;
		fe->fe_next = 0;
		ch->ch_head = fe;
	    }
	    else ch->ch_head = 0;
	    rdfeedchain_proliferate(ch, en->en_graphpools,
				    (t_gobj *)graph, key, size);
	}
    }
    return (ch);
}

t_rdfeedchain *riddle_useidlechain(t_riddle *rd, int sigoutno)
{
    return (riddle_usefeedchain(rd, sigoutno, rdps__idle, 1));
}

void rdfeedchain_feed(t_rdfeedchain *ch, int size, t_float *data)
{
    t_rdfeeder *fe;
    for (fe = ch->ch_head; fe; fe = fe->fe_next)
    {
	t_rdpicker *pi = fe->fe_picker;
	if (size > pi->pi_size)
	    size = pi->pi_size;
	memcpy(pi->pi_data, data, size * sizeof(*pi->pi_data));
    }
}

void rdfeedchain_feed1(t_rdfeedchain *ch, t_float v)
{
    t_rdfeeder *fe;
    for (fe = ch->ch_head; fe; fe = fe->fe_next)
	*fe->fe_picker->pi_data = v;
}

int riddle_isidle(t_riddle *rd)
{
    return (rd->rd_idlepicker && *rd->rd_idlepicker->pi_data > .5);
}

void riddle_updatedsp(void)
{
    t_rdenvironment *en = rdenvironment_provide();
    loud_warning((t_pd *)en, 0, "...trying to reconstruct the dsp chain");
    clock_delay(en->en_updatedspclock, 0);
}


/* rdremote: global named writers, global named readers,
   and private anonymous bidirectional buffers */


static t_rdremote *rdenvironment_getbuffer(t_rdenvironment *en, t_symbol *name)
{
    t_rdremote *re = en->en_writers;
    while (re)
    {
	if (re->re_name == name)
	    return (re);
	re = re->re_next;
    }
    return (0);
}

t_rdremote *rdremote_getwriter(t_rdremote *re)
{
    t_rdenvironment *en = rdenvironment_provide();
    return (rdenvironment_getbuffer(en, re->re_name));
}

t_rdremote *rdremote_nextreader(t_rdremote *re)
{
    while (re && !re->re_id)
	re = re->re_portlink;
    return (re);
}

int rdremote_getsourceblock(t_rdremote *re)
{
    if (re->re_owner && re->re_id > 0)
	return (riddle_getsourceblock(re->re_owner, -re->re_id));
    else
    {
	loudbug_bug("rdremote_getsourceblock");
	return (0);
    }
}

t_symbol *rdremote_getsourcelayout(t_rdremote *re, int *maxblockp)
{
    if (re->re_owner && re->re_id > 0)
	return (riddle_getsourcelayout(re->re_owner, -re->re_id, maxblockp));
    else
    {
	loudbug_bug("rdremote_getsourcelayout");
	return (0);
    }
}

int rdremote_getsourceflags(t_rdremote *re)
{
    if (re->re_owner && re->re_id > 0)
	return (riddle_getsourceflags(re->re_owner, -re->re_id));
    else
    {
	loudbug_bug("rdremote_getsourceflags");
	return (0);
    }
}

/* this call reallocates memory if necessary, so the caller should check
   for failures: the number of frames and/ot framesize may decrease 
   (the actually available framesize is returned by the call) */
/* LATER optionally use old contents by zero-padding, interpolating, etc. */
static int rdremote_setframesize(t_rdremote *re, int framesize)
{
    t_rdremote *reader;
    if (re->re_inidata == 0)
    {
	/* not a writer */
	loudbug_bug("rdremote_setframesize 1");
	return (0);
    }
    if (framesize <= 0)
    {
	if (re->re_owner)
	    framesize = re->re_owner->rd_graphblock;
	else
	{
	    loudbug_bug("rdremote_setframesize 2");
	    return (0);
	}
    }
    re->re_npoints = framesize * re->re_nframes;
    if (re->re_npoints > re->re_datasize)
    {
	int reqsize = re->re_npoints;
	/* LATER use grow_withdata() */
	re->re_data = grow_nodata(&re->re_npoints, &re->re_datasize,
				    re->re_data, RDREMOTE_INISIZE,
				    re->re_inidata, sizeof(*re->re_data));
	if (re->re_npoints != reqsize)
	{
	    re->re_nframes = re->re_npoints / framesize;
	    if (re->re_nframes < 1)
	    {
		loudbug_bug("rdremote_setframesize 3");
		re->re_nframes = 1;
		framesize = re->re_npoints;
	    }
	}
    }
    /* LATER convert old buffer's contents of re->re_framesize * re->re_nframes
       points into the new buffer of framesize * re->re_nframes points */
    memset(re->re_data, 0, re->re_npoints * sizeof(*re->re_data));
    re->re_phase = 0;  /* LATER adjust */
    re->re_maxphase = re->re_npoints - framesize;
    re->re_framesize = framesize;

    for (reader = re->re_readers; reader; reader = reader->re_next)
    {
	reader->re_nframes = re->re_nframes;
	reader->re_framesize = re->re_framesize;
	reader->re_npoints = re->re_npoints;
	reader->re_maxphase = re->re_maxphase;
	reader->re_data = re->re_data;
	reader->re_phase = 0;  /* LATER adjust */
    }
    return (framesize);
}

void rdremote_setoutblock(t_rdremote *re, int nblock)
{
    if (nblock = rdremote_setframesize(re, nblock))
    {
	t_rdremote *reader;
	for (reader = re->re_readers; reader; reader = reader->re_next)
	    if (reader->re_owner && reader->re_id > 0)
		riddle_setsourceblock(reader->re_owner, -reader->re_id,
				      re->re_framesize);
    }
}

void rdremote_setoutlayout(t_rdremote *re, t_symbol *pattern, int maxblock)
{
    if (maxblock = rdremote_setframesize(re, maxblock))
    {
	t_rdremote *reader;
	for (reader = re->re_readers; reader; reader = reader->re_next)
	    if (reader->re_owner && reader->re_id > 0)
		riddle_setsourcelayout(reader->re_owner, -reader->re_id,
				       pattern, re->re_framesize);
    }
}

void rdremote_setoutflags(t_rdremote *re, int flags)
{
    t_rdremote *reader;
    for (reader = re->re_readers; reader; reader = reader->re_next)
	if (reader->re_owner && reader->re_id > 0)
	    riddle_setsourceflags(reader->re_owner, -reader->re_id, flags);
}

void rdremote_reset(t_rdremote *re)
{
    if (re->re_inidata)
    {
	memset(re->re_data, 0, re->re_npoints * sizeof(*re->re_data));
	re->re_phase = 0;
    }
    else
    {
	t_rdremote *writer = rdremote_getwriter(re);
	if (writer)
	    re->re_phase = writer->re_phase;
	else
	    re->re_phase = 0;
    }
}

t_float *rdremote_gethead(t_rdremote *re)
{
    return (re->re_data + re->re_phase);
}

void rdremote_stephead(t_rdremote *re)
{
    re->re_phase += re->re_framesize;
    if (re->re_phase > re->re_maxphase)
	re->re_phase = 0;
}

void rdremote_movehead(t_rdremote *re, int nframes)
{
    if (re->re_nframes <= 0)
    {
	loudbug_bug("rdremote_movehead");
    }
    else if (nframes > 0)
    {
	if (nframes >= re->re_nframes)
	    nframes = re->re_nframes - 1;
	re->re_phase += nframes * re->re_framesize;
	while (re->re_phase > re->re_maxphase)
	    re->re_phase -= re->re_npoints;
    }
    else if (nframes < 0)
    {
	nframes = -nframes;
	if (nframes >= re->re_nframes)
	    nframes = re->re_nframes - 1;
	re->re_phase -= nframes * re->re_framesize;
	while (re->re_phase < 0)
	    re->re_phase += re->re_npoints;
    }
}

void rdremote_delayhead(t_rdremote *re, int nframes)
{
    if (re->re_inidata)
	loudbug_bug("rdremote_delayhead");  /* not a reader */
    else
    {
	t_rdremote *writer = rdremote_getwriter(re);
	if (writer)
	{
	    re->re_phase = writer->re_phase;
	    rdremote_movehead(re, -nframes);
	}
    }
}

static void rdremote_free(t_rdremote *re)
{
    if (re->re_inidata)
    {
	if (re->re_data != re->re_inidata)
	    freebytes(re->re_data, re->re_datasize * sizeof(*re->re_data));
	if (re->re_name)
	{
	    t_rdremote *reader;
	    t_rdenvironment *en = rdenvironment_provide();
	    /* remove from the environment */
	    if (re->re_next)
		re->re_next->re_prev = re->re_prev;
	    if (re->re_prev)
		re->re_prev->re_next = re->re_next;
	    else
		en->en_writers = re->re_next;
	    /* move all readers to the orphanage */
	    if (reader = re->re_readers)
	    {
		while (reader->re_next)
		    reader = reader->re_next;
		if (en->en_readers)
		    en->en_readers->re_prev = reader;
		reader->re_next = en->en_readers;
		en->en_readers = re->re_readers;
	    }
	}
    }
    else
    {
	if (re->re_name)
	{
	    /* remove from writer's list or orphanage */
	    if (re->re_next)
		re->re_next->re_prev = re->re_prev;
	    if (re->re_prev)
		re->re_prev->re_next = re->re_next;
	    else
	    {
		t_rdenvironment *en = rdenvironment_provide();
		t_rdremote *writer = rdenvironment_getbuffer(en, re->re_name);
		if (writer)
		    writer->re_readers = re->re_next;
		else
		    en->en_readers = re->re_next;
	    }
	}
    }
    freebytes(re, sizeof(*re));
}

void rdremote_freeports(t_rdremote *re)
{
    while (re)
    {
	t_rdremote *nxt = re->re_portlink;
	rdremote_free(re);
	re = nxt;
    }
}

/* FIXME do not rely on pd_new() callocing owner->rd_nremoteslots
   and owner->rd_remoteports to zero... one option is to traverse
   environment in riddle_new() after newfn call */
static t_rdremote *rdremote_newany(t_riddle *owner, t_symbol *name, int nframes)
{
    t_rdremote *re = (t_rdremote *)getbytes(sizeof(*re));
    if (name && !nframes)
    {
	owner->rd_nremoteslots++;
	re->re_id = owner->rd_nremoteslots;  /* starting from 1 */
    }
    else re->re_id = 0;
    re->re_name = 0;
    re->re_owner = owner;
    re->re_phase = 0;
    re->re_nframes = nframes;
    re->re_framesize = 0;
    re->re_npoints = 0;
    re->re_maxphase = 0;
    if (nframes)
    {
	re->re_datasize = RDREMOTE_INISIZE;
	re->re_inidata = getbytes(re->re_datasize * sizeof(*re->re_inidata));
	re->re_data = re->re_inidata;
    }
    else
    {
	re->re_datasize = 0;
	re->re_inidata = 0;
	re->re_data = 0;
    }
    re->re_readers = 0;
    re->re_prev = 0;
    re->re_next = 0;
    if (owner->rd_remoteports)
    {
	t_rdremote *prv = owner->rd_remoteports;
	while (prv->re_portlink)
	    prv = prv->re_portlink;
	prv->re_portlink = re;
    }
    else owner->rd_remoteports = re;
    re->re_portlink = 0;
    return (re);
}

t_rdremote *rdremote_newwriter(t_riddle *owner, t_symbol *name, int nframes)
{
    if (name && *name->s_name)
    {
	t_rdremote *re =
	    rdremote_newany(owner, name, (nframes > 1 ? nframes : 1));
	t_rdenvironment *en = rdenvironment_provide();
	if (rdenvironment_getbuffer(en, re->re_name))
	{
	    /* LATER accumulating writers case */
	    loud_error((t_pd *)owner, "duplicate buffer name \"%s\"",
		       re->re_name->s_name);
	    /* FIXME put on the namesakes queue */
	}
	else
	{
	    t_rdremote *reader;
	    /* store in the environment */
	    if (en->en_writers)
		en->en_writers->re_prev = re;
	    re->re_next = en->en_writers;
	    en->en_writers = re;
	    /* recover readers from the orphanage */
	    for (reader = en->en_readers; reader; reader = reader->re_next)
	    {
		if (reader->re_name == re->re_name)
		{
		    if (reader->re_next)
			reader->re_next->re_prev = reader->re_prev;
		    if (reader->re_prev)
			reader->re_prev->re_next = reader->re_next;
		    else
			en->en_readers = reader->re_next;
		    if (re->re_readers)
			re->re_readers->re_prev = reader;
		    reader->re_next = re->re_readers;
		    re->re_readers = reader;
		}
	    }
	}
	return (re);
    }
    else
    {
	loudbug_bug("rdremote_newwriter");
	return (0);
    }
}

t_rdremote *rdremote_newreader(t_riddle *owner, t_symbol *name)
{
    if (name && *name->s_name)
    {
	t_rdremote *re = rdremote_newany(owner, name, 0);
	t_rdenvironment *en = rdenvironment_provide();
	t_rdremote *writer = rdenvironment_getbuffer(en, name);
	if (writer)
	{
	    /* register to the writer */
	    if (writer->re_readers)
		writer->re_readers->re_prev = re;
	    re->re_next = writer->re_readers;
	    writer->re_readers = re;
	}
	else
	{
	    /* store in the orphanage */
	    if (en->en_readers)
		en->en_readers->re_prev = re;
	    re->re_next = en->en_readers;
	    en->en_readers = re;
	}
	return (re);
    }
    else
    {
	loudbug_bug("rdremote_newreader");
	return (0);
    }
}

t_rdremote *rdremote_newbuffer(t_riddle *owner, int nframes)
{
    return (rdremote_newany(owner, 0, (nframes > 1 ? nframes : 1)));
}
