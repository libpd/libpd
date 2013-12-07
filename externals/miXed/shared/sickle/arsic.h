/* Copyright (c) 2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __ARSIC_H__
#define __ARSIC_H__

typedef struct _arsic
{
    t_sic       s_sic;
    int         s_vecsize;   /* used also as a validation flag */
    int         s_nchannels;
    t_float   **s_vectors;
    t_symbol  **s_channames;
    int         s_nperfargs;
    t_int      *s_perfargs;
    t_symbol   *s_mononame;  /* used also as an 'ismono' flag */
    char       *s_stub;
    float       s_ksr;
    int         s_playable;
    int         s_minsize;
} t_arsic;

void arsic_clear(t_arsic *x);
void arsic_redraw(t_arsic *x);
void arsic_validate(t_arsic *x, int complain);
void arsic_check(t_arsic *x);
int arsic_getnchannels(t_arsic *x);
void arsic_setarray(t_arsic *x, t_symbol *s, int complain);
void arsic_setminsize(t_arsic *x, int i);

void arsic_dsp(t_arsic *x, t_signal **sp, t_perfroutine perf, int complain);
void *arsic_new(t_class *c, t_symbol *s,
		int nchannels, int nsigs, int nauxsigs);
void arsic_free(t_arsic *x);
void arsic_setup(t_class *c, void *dspfn, void *floatfn);

#endif
