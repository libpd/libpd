/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_delay written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_delay.h"

/* -------------------------- block_delay~ ------------------------------ */
static t_class *block_delay_tilde_class;

typedef struct _block_delay_tilde
{
	t_object	x_obj;
	t_float		*x_begmem;
	int				x_blocksize;
	t_float		x_msi;
} t_block_delay_tilde;

static t_int *block_delay_tilde_perform(t_int *w)
{
	t_float *in = (float *)(w[1]);
	t_float *out = (float *)(w[2]);
	t_block_delay_tilde *x = (t_block_delay_tilde *)(w[3]);
	int i, n = (t_int)(w[4]);
	t_float *rw_vec, f;

	rw_vec = x->x_begmem;
	for(i=0; i<n; i++)
	{
		f = in[i];
		out[i] = rw_vec[i];
		rw_vec[i] = f;
	}
	return(w+5);
}

static t_int *block_delay_tilde_perf8(t_int *w)
{
	t_float *in = (float *)(w[1]);
	t_float *out = (float *)(w[2]);
	t_block_delay_tilde *x = (t_block_delay_tilde *)(w[3]);
	int i, n = (t_int)(w[4]);
	t_float *rw_vec, f[8];

	rw_vec = x->x_begmem;
	while(n)
	{
		f[0] = in[0];
		f[1] = in[1];
		f[2] = in[2];
		f[3] = in[3];
		f[4] = in[4];
		f[5] = in[5];
		f[6] = in[6];
		f[7] = in[7];

		out[0] = rw_vec[0];
		out[1] = rw_vec[1];
		out[2] = rw_vec[2];
		out[3] = rw_vec[3];
		out[4] = rw_vec[4];
		out[5] = rw_vec[5];
		out[6] = rw_vec[6];
		out[7] = rw_vec[7];

		rw_vec[0] = f[0];
		rw_vec[1] = f[1];
		rw_vec[2] = f[2];
		rw_vec[3] = f[3];
		rw_vec[4] = f[4];
		rw_vec[5] = f[5];
		rw_vec[6] = f[6];
		rw_vec[7] = f[7];

		rw_vec += 8;
		in += 8;
		out += 8;
		n -= 8;
	}
	return(w+5);
}

static void block_delay_tilde_dsp(t_block_delay_tilde *x, t_signal **sp)
{
	int n = sp[0]->s_n;

	if(!x->x_blocksize)/*first time*/
		x->x_begmem = (t_float *)getbytes(n * sizeof(t_float));
	else if(x->x_blocksize != n)
		x->x_begmem = (t_float *)resizebytes(x->x_begmem, x->x_blocksize*sizeof(t_float), n*sizeof(t_float));
	x->x_blocksize = n;
	if(n&7)
		dsp_add(block_delay_tilde_perform, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
	else
		dsp_add(block_delay_tilde_perf8, 4, sp[0]->s_vec, sp[1]->s_vec, x, sp[0]->s_n);
}

static void *block_delay_tilde_new(void)
{
	t_block_delay_tilde *x = (t_block_delay_tilde *)pd_new(block_delay_tilde_class);

	x->x_blocksize = 0;
	x->x_begmem = (t_float *)0;
	outlet_new(&x->x_obj, &s_signal);
	x->x_msi = 0.0f;
	return (x);
}

static void block_delay_tilde_free(t_block_delay_tilde *x)
{
	if(x->x_begmem)
		freebytes(x->x_begmem, x->x_blocksize * sizeof(t_float));
}

void block_delay_tilde_setup(void)
{
	block_delay_tilde_class = class_new(gensym("block_delay~"), (t_newmethod)block_delay_tilde_new, (t_method)block_delay_tilde_free,
		sizeof(t_block_delay_tilde), 0, 0);
	CLASS_MAINSIGNALIN(block_delay_tilde_class, t_block_delay_tilde, x_msi);
	class_addmethod(block_delay_tilde_class, (t_method)block_delay_tilde_dsp, gensym("dsp"), 0);
//	class_sethelpsymbol(block_delay_tilde_class, gensym("iemhelp2/block_delay~-help"));
}
