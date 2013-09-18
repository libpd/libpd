/* Copyright (c) 2001-2003 krzYszcz and others.
 * For information on usage and redistribution, and for a DISCLAIMER OF ALL
 * WARRANTIES, see the file, "LICENSE.txt," in this distribution.  */

#ifndef __VEFL_H__
#define __VEFL_H__

typedef struct _vefl
{
    int        v_autoalloc;
    t_symbol  *v_name;
    t_glist   *v_glist;
    t_garray  *v_garray;
    int        v_size;
    t_float   *v_data;
    t_symbol  *v_type;
    t_clock   *v_clock;
    int        v_clockset;
    double     v_updtime;
} t_vefl;

t_float *vefl_get(t_symbol *name, int *vszp, int indsp, t_pd *complain);
t_vefl *vefl_new(t_symbol *name, int writable, t_glist *gl, t_garray *arr);
t_vefl *vefl_placement_new(t_vefl *vp, t_symbol *name,
			   int writable, t_glist *gl, t_garray *arr);
void vefl_free(t_vefl *vp);
int vefl_renew(t_vefl *vp, t_symbol *name, t_pd *complain);
void vefl_redraw(t_vefl *vp, float suppresstime);
void vefl_redraw_stop(t_vefl *vp);
void vefl_getbounds(t_vefl *vp, t_float *xminp, t_float *yminp,
		    t_float *xmaxp, t_float *ymaxp);
void vefl_setbounds(t_vefl *vp, t_float xmin, t_float ymin,
		    t_float xmax, t_float ymax);
void vefl_getrange(t_vefl *vp, t_float *yminp, t_float *ymaxp);

#endif
