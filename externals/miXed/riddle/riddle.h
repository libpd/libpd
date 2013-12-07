/* Copyright (c) 2007 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

/* these are the riddle external API declarations */

#ifndef __RIDDLE_H__
#define __RIDDLE_H__

EXTERN_STRUCT _riddle;
#define t_riddle  struct _riddle

EXTERN_STRUCT _rdprivate;
#define t_rdprivate  struct _rdprivate

EXTERN_STRUCT _rdsource;
#define t_rdsource  struct _rdsource

EXTERN_STRUCT _rdsink;
#define t_rdsink  struct _rdsink

EXTERN_STRUCT _rdpicker;
#define t_rdpicker  struct _rdpicker

EXTERN_STRUCT _rdfeedchain;
#define t_rdfeedchain  struct _rdfeedchain

EXTERN_STRUCT _rdremote;
#define t_rdremote  struct _rdremote

typedef void (*t_rdblockfn)(t_riddle *);
typedef void (*t_rddspfn)(t_riddle *, t_signal **);

struct _riddle
{
    t_sic        rd_sic;

    /* LATER rethink: indirection cost vs. abi stability */
    t_rdprivate *rd_private;

    /* designed for system-level control: block mismatches, etc.
       (user-level control possible via the '_idle' slot in graphpool) */
    int          rd_disabled;
    int          rd_wasdisabled;

    t_rdblockfn  rd_blockfn;
    t_rddspfn    rd_dspfn;

    int          rd_graphsr;
    int          rd_graphblock;

    int          rd_nsiginlets;
    int          rd_nremoteslots;
    t_rdsource  *rd_inslots;      /* nsiginlets + nremoteslots elements */
    t_rdsource  *rd_remoteslots;  /* == inslots + nsiginlets (readers only) */
    t_rdremote  *rd_remoteports;  /* the list of all remotes */

    int          rd_nsigoutlets;
    t_rdsink    *rd_outslots;     /* nsigoutlets elements */

    t_rdpicker  *rd_idlepicker;
};

#define RIDDLE_STRICTNESSMASK  1  /* if set: non-riddle sinks are rejected */

/* the main part of the API */

int riddle_getsourceblock(t_riddle *rd, int siginno);
t_symbol *riddle_getsourcelayout(t_riddle *rd, int siginno, int *maxblockp);
int riddle_getsourceflags(t_riddle *rd, int siginno);
/* or perhaps, IFBUILTIN, int inlet_getblock(t_inlet *)... */

void riddle_setoutblock(t_riddle *rd, int sigoutno, int newblock);
void riddle_setoutlayout(t_riddle *rd, int sigoutno,
			 t_symbol *pattern, int maxblock);
void riddle_setoutflags(t_riddle *rd, int sigoutno, int flags);
/* or perhaps, IFBUILTIN, void outlet_setblock(t_outlet *, int)... */

int riddle_checksourceblock(t_riddle *rd, int siginno, int reqblock);
int riddle_checksourcelayout(t_riddle *rd, int siginno,
			     t_symbol *reqpattern, int *maxblockp);

int riddle_isdisabled(t_riddle *rd);
void riddle_disable(t_riddle *rd);

/* this part is specific to the library implementation */

t_class *riddle_setup(t_symbol *name, t_newmethod newfn, t_method freefn,
		      size_t sz, t_method floatfn,
		      t_rdblockfn blockfn, t_rddspfn dspfn);

/* this part is very experimental: remote connections */

t_rdremote *rdremote_newwriter(t_riddle *owner, t_symbol *name, int nframes);
t_rdremote *rdremote_newreader(t_riddle *owner, t_symbol *name);
t_rdremote *rdremote_newbuffer(t_riddle *owner, int nframes);

int rdremote_getsourceblock(t_rdremote *re);
t_symbol *rdremote_getsourcelayout(t_rdremote *re, int *maxblockp);
int rdremote_getsourceflags(t_rdremote *re);

void rdremote_setoutblock(t_rdremote *re, int nblock);
void rdremote_setoutlayout(t_rdremote *re, t_symbol *pattern, int maxblock);
void rdremote_setoutflags(t_rdremote *re, int flags);

void rdremote_reset(t_rdremote *re);
t_float *rdremote_gethead(t_rdremote *re);
void rdremote_stephead(t_rdremote *re);
void rdremote_movehead(t_rdremote *re, int nframes);
void rdremote_delayhead(t_rdremote *re, int nframes);

t_rdpicker *rdpicker_attach(t_riddle *rd, t_symbol *key);
void rdpicker_detach(t_rdpicker *pi, t_riddle *rd);
t_float *rdpicker_pick(t_rdpicker *pi, int *sizep);
t_float rdpicker_pick1(t_rdpicker *pi);
int riddle_isidle(t_riddle *rd);

t_rdfeedchain *riddle_usefeedchain(t_riddle *rd,
				   int sigoutno, t_symbol *key, int size);
t_rdfeedchain *riddle_useidlechain(t_riddle *rd, int sigoutno);
void rdfeedchain_feed(t_rdfeedchain *ch, int size, t_float *data);
void rdfeedchain_feed1(t_rdfeedchain *ch, t_float v);

/* utilities */

void riddlebug_post(t_riddle *rd, char *pfx, char *fmt, ...);

int riddle_getsr(t_riddle *rd);
int riddle_getgraphblock(t_riddle *rd);

int riddle_erbfill(int nbands, int *buf, int nbins, int sr);

#endif
