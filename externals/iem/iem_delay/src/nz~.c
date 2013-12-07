/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_delay written by Thomas Musil (c) IEM KUG Graz Austria 2002 - 2006 */


#include "m_pd.h"
#include "iemlib.h"
#include "iem_delay.h"

/* -------------------------- nz_tilde~ ------------------------------ */
static t_class *nz_tilde_class;

typedef struct _nz_tilde
{
	t_object	x_obj;
	t_float		*x_begmem1;
	t_float		*x_begmem2;
	int				x_mallocsize;
	int				x_max_delay_samples;
	int				x_n_delays;
	int				x_writeindex;
	int				*x_del_samples;
	int				x_blocksize;
	t_float		**x_io;
	t_float		x_msi;
} t_nz_tilde;

static void nz_tilde_list(t_nz_tilde *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc == x->x_n_delays)
	{
		int i, delay, max=x->x_max_delay_samples;

		for(i=0; i<argc; i++)
		{
			delay = atom_getint(argv++);
			if(delay < 0)
				delay = 0;
			if(delay > max)
				delay = max;
			x->x_del_samples[i] = delay;
		}
	}
	else
		post("nz~-ERROR: list need %d delay-values between 0 and %d samples !!!!", x->x_n_delays, x->x_max_delay_samples);
}

static t_int *nz_tilde_perform(t_int *w)
{
	t_nz_tilde *x = (t_nz_tilde *)(w[1]);
	int n=(int)(w[2]);
	int num_dels=x->x_n_delays;
	t_float *in;
	t_float *out;
	int i, j;
	t_float *writevec1;
	t_float *writevec2;
	t_float *readvec;

	writevec2 = x->x_begmem2 + x->x_writeindex;
  writevec1 = x->x_begmem1 + x->x_writeindex;
	in = x->x_io[0];
	for(i=0; i<n; i++)
  {
		writevec2[i] = in[i];
		writevec1[i] = in[i];
  }

	for(j=0; j<num_dels; j++)
	{
		out = x->x_io[j+1];
		readvec = writevec2 - x->x_del_samples[j];
		for(i=0; i<n; i++)
			out[i] = readvec[i];
	}

	x->x_writeindex += n;
	if(x->x_writeindex >= x->x_mallocsize)
		x->x_writeindex -= x->x_mallocsize;

	return(w+3);
}

static t_int *nz_tilde_perf8(t_int *w)
{
	t_nz_tilde *x = (t_nz_tilde *)(w[1]);
	int n=(int)(w[2]);
	int num_dels=x->x_n_delays;
	t_float *in;
	t_float *out;
	int i, j;
	t_float *writevec1;
	t_float *writevec2;
	t_float *readvec;

	writevec2 = x->x_begmem2 + x->x_writeindex;
  writevec1 = x->x_begmem1 + x->x_writeindex;
	in = x->x_io[0];
	i = n;
	while(i)
	{
		writevec2[0] = in[0];
		writevec2[1] = in[1];
		writevec2[2] = in[2];
		writevec2[3] = in[3];
		writevec2[4] = in[4];
		writevec2[5] = in[5];
		writevec2[6] = in[6];
		writevec2[7] = in[7];

    writevec1[0] = in[0];
		writevec1[1] = in[1];
		writevec1[2] = in[2];
		writevec1[3] = in[3];
		writevec1[4] = in[4];
		writevec1[5] = in[5];
		writevec1[6] = in[6];
		writevec1[7] = in[7];

		writevec2 += 8;
    writevec1 += 8;
		in += 8;
		i -= 8;
	}

	for(j=0; j<num_dels; j++)
	{
		out = x->x_io[j+1];
		readvec = writevec2 - x->x_del_samples[j];
		i = n;
		while(i)
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
			i -= 8;
		}
	}

	x->x_writeindex += n;
	if(x->x_writeindex >= x->x_mallocsize)
		x->x_writeindex -= x->x_mallocsize;

	return(w+3);
}

static void nz_tilde_dsp(t_nz_tilde *x, t_signal **sp)
{
	int n = sp[0]->s_n;
	int i, j, max_samps, num_dels = x->x_n_delays + 1;

	if(!x->x_blocksize)/*first time*/
	{
		max_samps = x->x_max_delay_samples;
		i = max_samps / n;
		j = max_samps - i * n;
    /* allocate memory as a multiple of blocksize */
		if(j)
			max_samps = (i+1) * n;
		else
			max_samps = i * n;
//		post("malloc = %d, maxdel = %d", max_samps, x->x_max_delay_samples);
		x->x_mallocsize = max_samps;
		x->x_begmem1 = (t_float *)getbytes(2 * x->x_mallocsize * sizeof(t_float));
		x->x_begmem2 = x->x_begmem1 + x->x_mallocsize;
		x->x_writeindex = 0;
	}
	else if(x->x_blocksize != n)
	{
		max_samps = x->x_max_delay_samples;
		i = max_samps / n;
		j = max_samps - i * n;
		if(j)
			max_samps = (i+1) * n;
		else
			max_samps = i * n;
		x->x_begmem1 = (t_float *)resizebytes(x->x_begmem1, 2*x->x_mallocsize*sizeof(t_float), 2*max_samps*sizeof(t_float));
		x->x_mallocsize = max_samps;
		x->x_begmem2 = x->x_begmem1 + x->x_mallocsize;
		x->x_writeindex = 0;
	}
	x->x_blocksize = n;
	for(i=0; i<num_dels; i++)
		x->x_io[i] = sp[i]->s_vec;
	if(n&7)
		dsp_add(nz_tilde_perform, 2, x, n);
	else
		dsp_add(nz_tilde_perf8, 2, x, n);
}

static void *nz_tilde_new(t_floatarg n_delays, t_floatarg max_delay_samples)
{
	t_nz_tilde *x = (t_nz_tilde *)pd_new(nz_tilde_class);
	int i, n_out = (int)n_delays;
	int max_samps = (int)max_delay_samples;

	if(n_out < 1)
		n_out = 1;
	x->x_n_delays = n_out;
	if(max_samps < 1)
		max_samps = 1;
	x->x_max_delay_samples = max_samps;
	x->x_mallocsize = 0;
	x->x_begmem1 = (t_float *)0;
	x->x_begmem2 = (t_float *)0;
	x->x_writeindex = 0;
	x->x_blocksize = 0;
	x->x_io = (t_float **)getbytes((x->x_n_delays + 1) * sizeof(t_float *));
	x->x_del_samples = (int *)getbytes(x->x_n_delays * sizeof(int));
	for(i=0; i<n_out; i++)
	{
		outlet_new(&x->x_obj, &s_signal);
		x->x_del_samples[i] = 0;
	}
	x->x_msi = 0.0f;
	return (x);
}

static void nz_tilde_free(t_nz_tilde *x)
{
	freebytes(x->x_del_samples, x->x_n_delays * sizeof(int));
	freebytes(x->x_io, (x->x_n_delays + 1) * sizeof(t_float *));
	if(x->x_begmem1)
		freebytes(x->x_begmem1, 2 * x->x_mallocsize * sizeof(t_float));
}

void nz_tilde_setup(void)
{
	nz_tilde_class = class_new(gensym("nz~"), (t_newmethod)nz_tilde_new, (t_method)nz_tilde_free,
		sizeof(t_nz_tilde), 0, A_DEFFLOAT, A_DEFFLOAT, 0);
	CLASS_MAINSIGNALIN(nz_tilde_class, t_nz_tilde, x_msi);
	class_addlist(nz_tilde_class, (t_method)nz_tilde_list);
	class_addmethod(nz_tilde_class, (t_method)nz_tilde_dsp, gensym("dsp"), 0);
//	class_sethelpsymbol(nz_tilde_class, gensym("iemhelp2/nz~-help"));
}
