/* Copyright 2002 Benjamin R. Saylor
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include "m_pd.h"

static t_class *susloop_class;

static t_float pdsr;		/* pd's sample rate */

typedef struct _susloop {
	t_object x_obj;
	t_float a;		/* beginning of loop */
	t_float b;		/* end of loop */
	t_float startpos;	/* start offset */
	t_float pos;		/* position in sample */
	t_float direction;	/* for pingpong loops */
	t_float f;		/* dummy for when input is float instead of signal */
	void (*susloop_func)(t_int *x, t_float *in, t_float *out, int n);	/* can be forward or pingpong */
} t_susloop;

static void susloop_forward(t_int *x, t_float *in, t_float *out, int n)
{
	t_float sr;
	t_float p = ((t_susloop *)x)->pos;
	t_float a = ((t_susloop *)x)->a;
	t_float b = ((t_susloop *)x)->b;

	while (n--) {
		sr = *in;
		if (p > b) {
			p = a + (p - b);
		}
		*out = p;
		p += sr/pdsr;
		in++;
		out++;
	}

	((t_susloop *)x)->pos = p;
}

static void susloop_pingpong(t_int *x, t_float *in, t_float *out, int n)
{
	t_float sr;
	t_float p = ((t_susloop *)x)->pos;
	t_float a = ((t_susloop *)x)->a;
	t_float b = ((t_susloop *)x)->b;
	t_float d = ((t_susloop *)x)->direction;

	while (n--) {
		sr = *in;
		if (p > b) {
			p = b - (p - b);
			d = -1;
		} else if (p < a && d < 0) {
			p = a + (a - p);
			d = 1;
		}
		*out = p;
		p += d*(sr/pdsr);
		in++;
		out++;
	}

	((t_susloop *)x)->pos = p;
	((t_susloop *)x)->direction = d;
}

static t_int *susloop_perform(t_int *w)
{
	((t_susloop *)w[1])->susloop_func((t_int *)w[1], (t_float *)(w[2]), (t_float *)(w[3]), (int)(w[4]));
	return (w+5);
}

static void susloop_dsp(t_susloop *x, t_signal **sp)
{
	    dsp_add(susloop_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *susloop_new(t_symbol *s, int argc, t_atom *argv)
{
	t_susloop *x = (t_susloop *)pd_new(susloop_class);
	int looptype = 0;
	x->a = x->b = x->startpos = 0;
	x->direction = 1;
	x->susloop_func = susloop_forward;

	switch (argc) {
		case 4:
			x->startpos = atom_getfloat(argv+3);
		case 3:
			looptype = atom_getint(argv+2);
			if (looptype == 0)
				x->susloop_func = susloop_forward;
			else
				x->susloop_func = susloop_pingpong;
		case 2:
			x->a = atom_getfloat(argv);
			x->b = atom_getfloat(argv+1);
	}

	floatinlet_new(&x->x_obj, &x->a);
	floatinlet_new(&x->x_obj, &x->b);
	floatinlet_new(&x->x_obj, &x->startpos);
	outlet_new(&x->x_obj, gensym("signal"));
	return (x);
}

static void susloop_bang(t_susloop *x)
{
	x->pos = x->startpos;
	x->direction = 1;
}

static void susloop_setfunc(t_susloop *x, t_floatarg type)
{
	if (type == 1) {
		x->direction = 1;
		x->susloop_func = susloop_pingpong;
	} else
		x->susloop_func = susloop_forward;
}

void susloop_tilde_setup(void)
{
	pdsr = sys_getsr();
	susloop_class = class_new(gensym("susloop~"), (t_newmethod)susloop_new, 0, sizeof(t_susloop), 0, A_GIMME, 0);
	class_addbang(susloop_class, susloop_bang);
	/*
	class_addmethod(susloop_class, nullfn, gensym("signal"), 0);
	*/
	CLASS_MAINSIGNALIN(susloop_class, t_susloop, f);
	class_addmethod(susloop_class, (t_method)susloop_dsp, gensym("dsp"), 0);
	class_addmethod(susloop_class, (t_method)susloop_setfunc, gensym("type"), A_DEFFLOAT, 0);
}
