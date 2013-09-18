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

#ifdef _MSC_VER
#pragma warning( disable : 4244 )
#pragma warning( disable : 4305 )
#endif

static t_class *zhzxh_class;

typedef struct _zhzxh {
	t_object x_obj;
	t_float slope;
	t_float lastval;
	t_float srate;
	t_float f;
} t_zhzxh;

static t_int *zhzxh_perform(t_int *w)
{
	t_zhzxh *x = (t_zhzxh *)(w[1]);
	t_float *in = (t_float *)(w[2]);
	t_float *out = (t_float *)(w[3]);
	int n = (int)(w[4]);
	t_float f;

	t_float lastval = x->lastval;
	t_float delta = x->slope / x->srate;
	
	while (n--) {
		f = *(in++);
		if (lastval < f)
			*out = lastval + delta;
		else
			*out = lastval - delta;
		lastval = *(out++);
	}

	x->lastval = lastval;

	return (w+5);
}

static void zhzxh_dsp(t_zhzxh *x, t_signal **sp)
{
	dsp_add(zhzxh_perform, 4, x, sp[0]->s_vec, sp[1]->s_vec, sp[0]->s_n);
}

static void *zhzxh_new(t_symbol *s, int argc, t_atom *argv)
{
	t_zhzxh *x = (t_zhzxh *)pd_new(zhzxh_class);
	x->slope = 1;
	x->lastval = 0;
	x->srate = sys_getsr();
	floatinlet_new(&x->x_obj, &x->slope);
	outlet_new(&x->x_obj, gensym("signal"));

	return (x);
}

void zhzxh_tilde_setup(void)
{
	zhzxh_class = class_new(gensym("zhzxh~"), (t_newmethod)zhzxh_new, 0, sizeof(t_zhzxh), 0, A_GIMME, 0);
	CLASS_MAINSIGNALIN(zhzxh_class, t_zhzxh, f);
	class_addmethod(zhzxh_class, (t_method)zhzxh_dsp, gensym("dsp"), 0);
}
