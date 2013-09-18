/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_delay written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include "iem_delay.h"

/* -------------------------- n_delay2p_line_tilde~ ------------------------------ */
static t_class *n_delay2p_line_tilde_class;

t_float n_delay2p_line_tilde_256f[258];

typedef struct _n_delay2p_line_tilde
{
	t_object	x_obj;
	int				x_mallocsize;
	t_float		x_max_delay_ms;
	t_float		*x_begmem0;
	t_float		*x_begmem1;
	int				x_writeindex;
	int				x_n_delays;
	int				*x_del_samp256_end;
	int				*x_del_samp256_cur;
	int				*x_inc256;
	int				*x_biginc256;
	int				x_blocksize;
	t_float		x_sr;
	t_float		x_ms2tick;
	t_float		x_ms2samples256;
	t_float		x_interpol_ms;
	int				x_interpol_ticks;
	int				x_ticksleft;
	int				x_old;
	int				x_retarget;
	t_float		**x_io;
	t_float		x_msi;
} t_n_delay2p_line_tilde;

static void n_delay2p_line_tilde_init_f(t_n_delay2p_line_tilde *x)
{
	if(n_delay2p_line_tilde_256f[257] == 0.0f)
	{
		int i;

		for(i=0; i<257; i++)
		{
			n_delay2p_line_tilde_256f[i] = (t_float)i / 256.0f;
		}
		n_delay2p_line_tilde_256f[257] = 1.0f;
	}
}

static void n_delay2p_line_tilde_list(t_n_delay2p_line_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc == x->x_n_delays)
	{
		int i;
		t_float delay, max=x->x_max_delay_ms;

		if(x->x_interpol_ms <= 0.0f)
			x->x_ticksleft = x->x_retarget = 0;
		else
			x->x_retarget = 1;
		for(i=0; i<argc; i++)
		{
			delay = atom_getfloat(argv++);
			if(delay < 0.0f)
				delay = 0.0f;
			if(delay > max)
				delay = max;
			if(x->x_interpol_ms <= 0.0f)
				x->x_del_samp256_end[i] = x->x_del_samp256_cur[i] = (int)(x->x_ms2samples256 * delay + 0.5f) + 127;
			else
				x->x_del_samp256_end[i] = (int)(x->x_ms2samples256 * delay  + 0.5f) + 127;
		}
	}
}

static void n_delay2p_line_tilde_time(t_n_delay2p_line_tilde *x, t_floatarg interpol_ms)
{
	if(interpol_ms < 0.0f)
		interpol_ms = 0.0f;
	x->x_interpol_ms = interpol_ms;
	x->x_interpol_ticks = (int)(x->x_ms2tick * interpol_ms);
}

static void n_delay2p_line_tilde_stop(t_n_delay2p_line_tilde *x)
{
	int i, n=x->x_n_delays;

	for(i=0; i<n; i++)
		x->x_del_samp256_end[i] = x->x_del_samp256_cur[i];
	x->x_ticksleft = x->x_retarget = 0;
}

static t_int *n_delay2p_line_tilde_perform(t_int *w)
{
	t_n_delay2p_line_tilde *x = (t_n_delay2p_line_tilde *)(w[1]);
	int hn=(int)(w[2]);
	int nout=x->x_n_delays;
	t_float *in;
	t_float *out;
	int writeindex = x->x_writeindex;
	int i, j, n, fractindex;
	int malloc_samples = x->x_mallocsize;
	t_float *begvec0 = x->x_begmem0;
	t_float *begvec1 = x->x_begmem1;
	t_float *writevec;
	t_float *readvec;
	t_float fract;
	int del256, inc256;

	writevec = begvec0 + writeindex;
	in=x->x_io[0];
	n = hn;
	while(n--)
	{
		*writevec++ = *in++;
	}
	writevec = begvec1 + writeindex;
	in=x->x_io[0];
	n = hn;
	while(n--)
	{
		*writevec++ = *in++;
	}

	if(x->x_retarget)
	{
		int nticks = x->x_interpol_ticks;

		if(!nticks)
			nticks = 1;
		x->x_ticksleft = nticks;
		for(j=0; j<nout; j++)
		{
			x->x_biginc256[j] = (x->x_del_samp256_end[j] - x->x_del_samp256_cur[j]) / nticks;
			x->x_inc256[j] = x->x_biginc256[j] / x->x_blocksize;
		}
		x->x_retarget = 0;
	}

	if(x->x_ticksleft)
	{
		for(j=0; j<nout; j++)
		{
			inc256 = x->x_inc256[j];
			del256 = x->x_del_samp256_cur[j];
			out = x->x_io[j+1];
			for(i=0; i<hn; i++)
			{
				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				readvec = begvec1 + writeindex - (del256 >> 8) + i;
				*out++ = readvec[0] - (readvec[0] - readvec[-1])*fract;
				del256 += inc256;
			}
			x->x_del_samp256_cur[j] += x->x_biginc256[j];
		}
		x->x_ticksleft--;
	}
	else
	{
		for(j=0; j<nout; j++)
		{
			del256 = x->x_del_samp256_cur[j] = x->x_del_samp256_end[j];
			readvec = begvec1 + writeindex - (del256 >> 8);
			out = x->x_io[j+1];
			n = hn;
			while(n--)
			{
				*out++ = *readvec++;
			}
		}
	}
	writeindex += hn;
	if(writeindex >= malloc_samples)
	{
		writeindex -= malloc_samples;
	}
	x->x_writeindex = writeindex;
	return(w+3);
}

static t_int *n_delay2p_line_tilde_perf8(t_int *w)
{
	t_n_delay2p_line_tilde *x = (t_n_delay2p_line_tilde *)(w[1]);
	int hn=(int)(w[2]);
	int nout=x->x_n_delays;
	t_float *in;
	t_float *out;
	int writeindex = x->x_writeindex;
	int i, j, k, n, fractindex;
	int malloc_samples = x->x_mallocsize;
	t_float *begvec0 = x->x_begmem0;
	t_float *begvec1 = x->x_begmem1;
	t_float *writevec;
	t_float *readvec;
	t_float fract;
	int del256, inc256;

//	post("writevec = %d",writeindex);
	writevec = begvec0 + writeindex;
	in=x->x_io[0];
	n = hn;
	while(n)
	{
		writevec[0] = in[0];
		writevec[1] = in[1];
		writevec[2] = in[2];
		writevec[3] = in[3];
		writevec[4] = in[4];
		writevec[5] = in[5];
		writevec[6] = in[6];
		writevec[7] = in[7];

		writevec += 8;
		n -= 8;
		in += 8;
	}
	writevec = begvec1 + writeindex;
	in=x->x_io[0];
	n = hn;
	while(n)
	{
		writevec[0] = in[0];
		writevec[1] = in[1];
		writevec[2] = in[2];
		writevec[3] = in[3];
		writevec[4] = in[4];
		writevec[5] = in[5];
		writevec[6] = in[6];
		writevec[7] = in[7];

		writevec += 8;
		n -= 8;
		in += 8;
	}

	if(x->x_retarget)
	{
		int nticks = x->x_interpol_ticks;

		if(!nticks)
			nticks = 1;
		x->x_ticksleft = nticks;
		for(j=0; j<nout; j++)
		{
			x->x_biginc256[j] = (x->x_del_samp256_end[j] - x->x_del_samp256_cur[j]) / nticks;
			x->x_inc256[j] = x->x_biginc256[j] / x->x_blocksize;
		}
		x->x_retarget = 0;
	}

	if(x->x_ticksleft)
	{
		for(j=0; j<nout; j++)
		{
			inc256 = x->x_inc256[j];
			del256 = x->x_del_samp256_cur[j];
			out = x->x_io[j+1];
			readvec = begvec1 + writeindex;
			for(i=0; i<hn; i+=8)
			{
				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[0] = readvec[0-k] - (readvec[0-k] - readvec[-1-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[1] = readvec[1-k] - (readvec[1-k] - readvec[0-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[2] = readvec[2-k] - (readvec[2-k] - readvec[1-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[3] = readvec[3-k] - (readvec[3-k] - readvec[2-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[4] = readvec[4-k] - (readvec[4-k] - readvec[3-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[5] = readvec[5-k] - (readvec[5-k] - readvec[4-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[6] = readvec[6-k] - (readvec[6-k] - readvec[5-k])*fract;
				del256 += inc256;

				fractindex = del256 & 0xff;
				fract = n_delay2p_line_tilde_256f[fractindex];
				k = del256 >> 8;
				out[7] = readvec[7-k] - (readvec[7-k] - readvec[6-k])*fract;
				del256 += inc256;

				out += 8;
				readvec += 8;
			}
			x->x_del_samp256_cur[j] += x->x_biginc256[j];
		}
		x->x_ticksleft--;
	}
	else
	{
		for(j=0; j<nout; j++)
		{
			del256 = x->x_del_samp256_cur[j] = x->x_del_samp256_end[j];
			readvec = begvec1 + writeindex - (del256 >> 8);
			out = x->x_io[j+1];
			n = hn;
			while(n)
			{
				out[0] = readvec[0];
				out[1] = readvec[1];
				out[2] = readvec[2];
				out[3] = readvec[3];
				out[4] = readvec[4];
				out[5] = readvec[5];
				out[6] = readvec[6];
				out[7] = readvec[7];
				out += 8;
				readvec += 8;
				n -= 8;
			}
		}
	}
	writeindex += hn;
	if(writeindex >= malloc_samples)
	{
		writeindex -= malloc_samples;
	}
	x->x_writeindex = writeindex;
	return(w+3);
}

static void n_delay2p_line_tilde_dsp(t_n_delay2p_line_tilde *x, t_signal **sp)
{
	int n = sp[0]->s_n;
	int i, nd = x->x_n_delays + 1;

	if(!x->x_blocksize)/*first time*/
	{
		int nsamps = x->x_max_delay_ms * (t_float)sp[0]->s_sr * 0.001f;

		if(nsamps < 1)
			nsamps = 1;
		nsamps += ((- nsamps) & (n - 1));
		nsamps += n;
		x->x_mallocsize = nsamps;
		x->x_begmem0 = (t_float *)getbytes(2 * x->x_mallocsize * sizeof(t_float));
		x->x_begmem1 = x->x_begmem0 + x->x_mallocsize;
		x->x_writeindex = n;
	}
	else if((x->x_blocksize != n) || ((t_float)sp[0]->s_sr != x->x_sr))
	{
		int nsamps = x->x_max_delay_ms * (t_float)sp[0]->s_sr * 0.001f;

		if(nsamps < 1)
			nsamps = 1;
		nsamps += ((- nsamps) & (n - 1));
		nsamps += n;
		x->x_begmem0 = (t_float *)resizebytes(x->x_begmem0, 2*x->x_mallocsize*sizeof(t_float), 2*nsamps*sizeof(t_float));
		x->x_mallocsize = nsamps;
		x->x_begmem1 = x->x_begmem0 + x->x_mallocsize;
		if(x->x_writeindex >= nsamps)
			x->x_writeindex -= nsamps;
	}
	x->x_blocksize = n;
	x->x_ms2tick = 0.001f * (t_float)sp[0]->s_sr / (t_float)n;
	x->x_ms2samples256 = 0.256f * (t_float)sp[0]->s_sr;
	x->x_interpol_ticks = (int)(x->x_ms2tick * x->x_interpol_ms);
	for(i=0; i<nd; i++)
		x->x_io[i] = sp[i]->s_vec;
	if(n&7)
		dsp_add(n_delay2p_line_tilde_perform, 2, x, n);
	else
		dsp_add(n_delay2p_line_tilde_perf8, 2, x, n);
}

static void *n_delay2p_line_tilde_new(t_floatarg fout, t_floatarg delay_ms, t_floatarg interpol_ms)
{
	t_n_delay2p_line_tilde *x = (t_n_delay2p_line_tilde *)pd_new(n_delay2p_line_tilde_class);
	int i, n_out = (int)fout;
	int nsamps = delay_ms * sys_getsr() * 0.001f;

	if(n_out < 1)
		n_out = 1;
	x->x_n_delays = n_out;
	x->x_max_delay_ms = delay_ms;
	if(nsamps < 1)
		nsamps = 1;
	nsamps += ((- nsamps) & (DELLINE_DEF_VEC_SIZE - 1));
	nsamps += DELLINE_DEF_VEC_SIZE;
	x->x_mallocsize = nsamps;
	x->x_begmem0 = (t_float *)getbytes(2 * x->x_mallocsize * sizeof(t_float));
	x->x_begmem1 = x->x_begmem0 + x->x_mallocsize;
	x->x_writeindex = DELLINE_DEF_VEC_SIZE;
	x->x_blocksize = 0;
	x->x_sr = 0.0f;
	if(interpol_ms < 0.0f)
		interpol_ms = 0.0f;
	x->x_interpol_ms = interpol_ms;
	x->x_io = (t_float **)getbytes((x->x_n_delays + 1) * sizeof(t_float *));
	for(i=0; i<n_out; i++)
		outlet_new(&x->x_obj, &s_signal);
	x->x_del_samp256_end = (int *)getbytes(x->x_n_delays * sizeof(int));
	x->x_del_samp256_cur = (int *)getbytes(x->x_n_delays * sizeof(int));
	x->x_inc256 = (int *)getbytes(x->x_n_delays * sizeof(int));
	x->x_biginc256 = (int *)getbytes(x->x_n_delays * sizeof(int));
	x->x_ticksleft = x->x_retarget = 0;
	for(i=0; i<n_out; i++)
	{
		x->x_del_samp256_cur[i] = x->x_del_samp256_end[i] = 0;
		x->x_inc256[i] = x->x_biginc256[i] = 0;
	}
	x->x_interpol_ticks = 0;
	x->x_msi = 0.0f;
	n_delay2p_line_tilde_init_f(x);
	return (x);
}

static void n_delay2p_line_tilde_free(t_n_delay2p_line_tilde *x)
{
	freebytes(x->x_del_samp256_end, x->x_n_delays * sizeof(int));
	freebytes(x->x_del_samp256_cur, x->x_n_delays * sizeof(int));
	freebytes(x->x_inc256, x->x_n_delays * sizeof(int));
	freebytes(x->x_biginc256, x->x_n_delays * sizeof(int));
	freebytes(x->x_io, (x->x_n_delays + 1) * sizeof(t_float *));
	freebytes(x->x_begmem0, 2 * x->x_mallocsize * sizeof(t_float));
}

void n_delay2p_line_tilde_setup(void)
{
	n_delay2p_line_tilde_class = class_new(gensym("n_delay2p_line~"), (t_newmethod)n_delay2p_line_tilde_new, (t_method)n_delay2p_line_tilde_free,
		sizeof(t_n_delay2p_line_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(n_delay2p_line_tilde_class, t_n_delay2p_line_tilde, x_msi);
	class_addlist(n_delay2p_line_tilde_class, (t_method)n_delay2p_line_tilde_list);
	class_addmethod(n_delay2p_line_tilde_class, (t_method)n_delay2p_line_tilde_dsp, gensym("dsp"), 0);
	class_addmethod(n_delay2p_line_tilde_class, (t_method)n_delay2p_line_tilde_stop, gensym("stop"), 0);
	class_addmethod(n_delay2p_line_tilde_class, (t_method)n_delay2p_line_tilde_time, gensym("time"), A_FLOAT, 0);
//	class_sethelpsymbol(n_delay2p_line_tilde_class, gensym("iemhelp2/n_delay2p_line~-help"));
}
