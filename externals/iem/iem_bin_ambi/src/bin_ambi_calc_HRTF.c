/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_bin_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_bin_ambi.h"
#include <math.h>
#include <stdio.h>



/* -------------------------- bin_ambi_calc_HRTF ------------------------------ */
/*
 ** berechnet ein reduziertes Ambisonic-Decoder-Set in die HRTF-Spektren **
 ** Inputs: ls + Liste von 3 floats: Index [1 .. 16] + Elevation [-90 .. +90 degree] + Azimut [0 .. 360 degree] **
 ** Inputs: calc_inv **
 ** Inputs: load_HRIR + float index1..16 **
 ** Outputs: List of 2 symbols: left-HRIR-File-name + HRIR-table-name **
 ** Inputs: calc_reduced **
 ** "output" ...  writes the HRTF into tables **
 **  **
 **  **
 ** setzt voraus , dass die HRIR-tabele-names von LS1_L_HRIR .. LS16_L_HRIR heissen und existieren **
 ** setzt voraus , dass die HRTF-tabele-names von LS1_HRTF_re .. LS16_HRTF_re heissen und existieren **
 ** setzt voraus , dass die HRTF-tabele-names von LS1_HRTF_im .. LS16_HRTF_im heissen und existieren **
 */

typedef struct _bin_ambi_calc_HRTF
{
	t_object			x_obj;
	t_atom				x_at[2];
	int						x_n_ls;
	int						x_fftsize;
	int						*x_delta;
	int						*x_phi;
	BIN_AMBI_COMPLEX	*x_spec;
	BIN_AMBI_COMPLEX	*x_sin_cos;
	iemarray_t			*x_beg_fade_out_hrir;
	t_float				*x_beg_hrir;
	iemarray_t			**x_beg_hrtf_re;
	iemarray_t			**x_beg_hrtf_im;
	t_symbol			**x_hrir_filename;
	t_symbol			**x_s_hrir;
	t_symbol			**x_s_hrtf_re;
	t_symbol			**x_s_hrtf_im;
	t_symbol			*x_s_fade_out_hrir;
	double				x_pi_over_180;
} t_bin_ambi_calc_HRTF;

static t_class *bin_ambi_calc_HRTF_class;

static void bin_ambi_calc_HRTF_init_cos(t_bin_ambi_calc_HRTF *x)
{
	int i, fftsize = x->x_fftsize;
	t_float f, g;
	BIN_AMBI_COMPLEX *sincos = x->x_sin_cos;

	g = 2.0f * 3.1415926538f / (t_float)fftsize;
	for(i=0; i<fftsize; i++)
	{
		f = g * (t_float)i;
		(*sincos).real = cos(f);
		(*sincos).imag = -sin(f);/*FFT*/
		sincos++;
	}
}

static void bin_ambi_calc_HRTF_quant(t_bin_ambi_calc_HRTF *x, double *delta_deg2rad, double *phi_deg2rad, int index)
{
	double q = 1.0;
	double d = *delta_deg2rad;
	double p = *phi_deg2rad;
	int i;

	if(d < -40.0)
		d = -40.0;
	if(d > 90.0)
		d = 90.0;
	while(p < 0.0)
		p += 360.0;
	while(p >= 360.0)
		p -= 360.0;

	if(d < -35.0)
	{
		*delta_deg2rad = -40.0;
		q = 360.0 / 56.0;
	}
	else if(d < -25.0)
	{
		*delta_deg2rad = -30.0;
		q = 6.0;
	}
	else if(d < -15.0)
	{
		*delta_deg2rad = -20.0;
		q = 5.0;
	}
	else if(d < -5.0)
	{
		*delta_deg2rad = -10.0;
		q = 5.0;
	}
	else if(d < 5.0)
	{
		*delta_deg2rad = 0.0;
		q = 5.0;
	}
	else if(d < 15.0)
	{
		*delta_deg2rad = 10.0;
		q = 5.0;
	}
	else if(d < 25.0)
	{
		*delta_deg2rad = 20.0;
		q = 5.0;
	}
	else if(d < 35.0)
	{
		*delta_deg2rad = 30.0;
		q = 6.0;
	}
	else if(d < 45.0)
	{
		*delta_deg2rad = 40.0;
		q = 360.0 / 56.0;
	}
	else if(d < 55.0)
	{
		*delta_deg2rad = 50.0;
		q = 8.0;
	}
	else if(d < 65.0)
	{
		*delta_deg2rad = 60.0;
		q = 10.0;
	}
	else if(d < 75.0)
	{
		*delta_deg2rad = 70.0;
		q = 15.0;
	}
	else if(d < 85.0)
	{
		*delta_deg2rad = 80.0;
		q = 30.0;
	}
	else
	{
		*delta_deg2rad = 90.0;
		q = 360.0;
	}

	p /= q;
	i = (int)(p + 0.499999);
	p = (double)i * q;
	i = (int)(p + 0.499999);
	while(i >= 360)
		i -= 360;
	p = (double)i;
	*phi_deg2rad = p;

	x->x_delta[index] = (int)(*delta_deg2rad);
	x->x_phi[index] = i;

	*delta_deg2rad *= x->x_pi_over_180;
	*phi_deg2rad *= x->x_pi_over_180;
}

static void bin_ambi_calc_HRTF_do_2d(t_bin_ambi_calc_HRTF *x, int argc, t_atom *argv)
{
	double delta=0.0, phi;
	int index;
	int n_ls = x->x_n_ls;

	if(argc < 2)
	{
		post("bin_ambi_calc_HRTF ERROR: ls-input needs 1 index and 1 angle: ls_index + phi [degree]");
		return;
	}
	index = (int)atom_getint(argv++) - 1;
	phi = (double)atom_getfloat(argv);

	if(index < 0)
		index = 0;
	if(index >= n_ls)
		index = n_ls - 1;

	bin_ambi_calc_HRTF_quant(x, &delta, &phi, index);
}

static void bin_ambi_calc_HRTF_do_3d(t_bin_ambi_calc_HRTF *x, int argc, t_atom *argv)
{
	double delta, phi;
	int index;
	int n_ls=x->x_n_ls;

	if(argc < 3)
	{
		post("bin_ambi_calc_HRTF ERROR: ls-input needs 1 index and 2 angles: ls index + delta [degree] + phi [degree]");
		return;
	}
	index = (int)atom_getint(argv++) - 1;
	delta = atom_getfloat(argv++);
	phi = atom_getfloat(argv);

	if(index < 0)
		index = 0;
	if(index >= n_ls)
		index = n_ls - 1;
	
	bin_ambi_calc_HRTF_quant(x, &delta, &phi, index);
}

static void bin_ambi_calc_HRTF_ls(t_bin_ambi_calc_HRTF *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc >= 3)
		bin_ambi_calc_HRTF_do_3d(x, argc, argv);
	else
		bin_ambi_calc_HRTF_do_2d(x, argc, argv);
}

static void bin_ambi_calc_HRTF_check_fade_out(t_bin_ambi_calc_HRTF *x)
{
	t_garray *a;
	int npoints;
	iemarray_t *fadevec;

	if(x->x_beg_fade_out_hrir == 0)
	{
		if (!(a = (t_garray *)pd_findbyclass(x->x_s_fade_out_hrir, garray_class)))
			error("%s: no such array", x->x_s_fade_out_hrir->s_name);
		else if (!iemarray_getarray(a, &npoints, &fadevec))
			error("%s: bad template for bin_ambi_calc_HRTF", x->x_s_fade_out_hrir->s_name);
		else if (npoints < x->x_fftsize)
			error("%s: bad array-size: %d", x->x_s_fade_out_hrir->s_name, npoints);
		else
			x->x_beg_fade_out_hrir = fadevec;
	}
}

/*
x_prod:
n_ambi  columns;
n_ambi    rows;
*/

static void bin_ambi_calc_HRTF_load_HRIR(t_bin_ambi_calc_HRTF *x, t_float findex)
{
	int index=(int)findex - 1;
	int p;
	char buf[60];

	if(index < 0)
		index = 0;
	if(index >= x->x_n_ls)
		index = x->x_n_ls - 1;

	p = x->x_phi[index];
	
	if(p)/*change*/
		p = 360 - p;
	
	if(p < 10)
		sprintf(buf, "L%de00%da.wav", x->x_delta[index], p);
	else if(p < 100)
		sprintf(buf, "L%de0%da.wav", x->x_delta[index], p);
	else
		sprintf(buf, "L%de%da.wav", x->x_delta[index], p);
	x->x_hrir_filename[index] = gensym(buf);

	SETSYMBOL(x->x_at, x->x_hrir_filename[index]);
	SETSYMBOL(x->x_at+1, x->x_s_hrir[index]);
	outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
}

static void bin_ambi_calc_HRTF_check_arrays(t_bin_ambi_calc_HRTF *x, t_float findex)
{
	int index=(int)findex - 1;
	int j, k, n;
	int fftsize = x->x_fftsize;
	int fs2=fftsize/2;
	t_garray *a;
	int npoints;
	t_float *vec;
	iemarray_t *vec_fade_out_hrir;
	iemarray_t *vec_hrir, *vec_hrtf_re, *vec_hrtf_im;
	t_symbol *hrir, *hrtf_re, *hrtf_im;
	t_float decr, sum;

	if(index < 0)
		index = 0;
	if(index >= x->x_n_ls)
		index = x->x_n_ls - 1;

	hrir = x->x_s_hrir[index];
	hrtf_re = x->x_s_hrtf_re[index];
	hrtf_im = x->x_s_hrtf_im[index];

	if (!(a = (t_garray *)pd_findbyclass(hrtf_re, garray_class)))
		error("%s: no such array", hrtf_re->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrtf_re))
		error("%s: bad template for bin_ambi_calc_HRTF", hrtf_re->s_name);
	else if (npoints < fftsize)
		error("%s: bad array-size: %d", hrtf_re->s_name, npoints);
	else if (!(a = (t_garray *)pd_findbyclass(hrtf_im, garray_class)))
		error("%s: no such array", hrtf_im->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrtf_im))
		error("%s: bad template for bin_ambi_calc_HRTF", hrtf_im->s_name);
	else if (npoints < fftsize)
		error("%s: bad array-size: %d", hrtf_im->s_name, npoints);
	else if (!(a = (t_garray *)pd_findbyclass(hrir, garray_class)))
		error("%s: no such array", hrir->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrir))
		error("%s: bad template for bin_ambi_calc_HRTF", hrir->s_name);
	else
	{
		x->x_beg_hrtf_re[index] = vec_hrtf_re;
		x->x_beg_hrtf_im[index] = vec_hrtf_im;

		if(npoints < fftsize)
		{
			post("warning: %s-array-size: %d", hrir->s_name, npoints);
		}
		vec = x->x_beg_hrir;
		vec += index * fftsize;
	
		if(x->x_beg_fade_out_hrir)
		{
			vec_fade_out_hrir = x->x_beg_fade_out_hrir;
			for(j=0; j<fs2; j++)
			  vec[j] = iemarray_getfloat(vec_hrir,j)*iemarray_getfloat(vec_fade_out_hrir,j);
		}
		else
		{
			post("no HRIR-fade-out-window found");
			n = fs2 * 3;
			n /= 4;
			for(j=0; j<n; j++)
			  vec[j] = iemarray_getfloat(vec_hrir,j);
			sum = 1.0f;
			decr = 4.0f / (t_float)fs2;
			for(j=n, k=0; j<fs2; j++, k++)
			{
				sum -= decr;
				vec[j] = iemarray_getfloat(vec_hrir,j) * sum;
			}
		}
	}
}

static void bin_ambi_calc_HRTF_calc_fft(t_bin_ambi_calc_HRTF *x, t_float findex)
{
	int index=(int)findex - 1;
	int i, j, k, w_index, w_inc, i_inc, v_index, fs1, fs2;
	int fftsize = x->x_fftsize;
	BIN_AMBI_COMPLEX old1, old2, w;
	BIN_AMBI_COMPLEX *sincos = x->x_sin_cos;
	BIN_AMBI_COMPLEX *val = x->x_spec;
	t_float *vec_hrir;
	iemarray_t *vec_hrtf_re, *vec_hrtf_im;
	int n_ls = x->x_n_ls;

	if(index < 0)
		index = 0;
	if(index >= n_ls)
		index = n_ls - 1;

	vec_hrtf_re = x->x_beg_hrtf_re[index];
	vec_hrtf_im = x->x_beg_hrtf_im[index];

	vec_hrir = x->x_beg_hrir;
	vec_hrir += index * fftsize;
	for(k=0; k<fftsize; k++)
	{
		val[k].real = vec_hrir[k];
		val[k].imag = 0.0f;
	}

	fs1 = fftsize - 1;
	fs2 = fftsize / 2;
	i_inc = fs2;
	w_inc = 1;
	for(i=1; i<fftsize; i<<=1)
	{
		v_index = 0;
		for(j=0; j<i; j++)
		{
			w_index = 0;
			for(k=0; k<i_inc; k++)
			{
				old1 = val[v_index];
				old2 = val[v_index+i_inc];
				w = sincos[w_index];
				val[v_index+i_inc].real = (old1.real-old2.real)*w.real - (old1.imag-old2.imag)*w.imag;
				val[v_index+i_inc].imag = (old1.imag - old2.imag)*w.real + (old1.real-old2.real)*w.imag;
				val[v_index].real = old1.real+old2.real;
				val[v_index].imag = old1.imag+old2.imag;
				w_index += w_inc;
				v_index++;
			}
			v_index += i_inc;
		}
		w_inc <<= 1;
		i_inc >>= 1;
	}

	j = 0;
	for(i=1;i<fs1;i++)
	{
		k = fs2;
		while(k <= j)
		{
			j = j - k;
			k >>= 1;
		}
		j = j + k;
		if(i < j)
		{
			old1 = val[j];
			val[j] = val[i];
			val[i] = old1;
		}
	}
  iemarray_setfloat(vec_hrtf_re, 0, val[0].real);
  for(i = 1; i<fs2; i++)
  {
    iemarray_setfloat(vec_hrtf_re, i, 2.0f*val[i].real);
    iemarray_setfloat(vec_hrtf_im, i, 2.0f*val[i].imag);
  }
  iemarray_setfloat(vec_hrtf_re, fs2, val[fs2].real);
  iemarray_setfloat(vec_hrtf_im, fs2, 0.0f);
}

static void bin_ambi_calc_HRTF_free(t_bin_ambi_calc_HRTF *x)
{
	freebytes(x->x_hrir_filename, x->x_n_ls * sizeof(t_symbol *));
	freebytes(x->x_s_hrir, x->x_n_ls * sizeof(t_symbol *));
	freebytes(x->x_s_hrtf_re, x->x_n_ls * sizeof(t_symbol *));
	freebytes(x->x_s_hrtf_im, x->x_n_ls * sizeof(t_symbol *));

	freebytes(x->x_delta, x->x_n_ls * sizeof(int));
	freebytes(x->x_phi, x->x_n_ls * sizeof(int));

	freebytes(x->x_spec, x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));
	freebytes(x->x_sin_cos, x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));

	freebytes(x->x_beg_hrir, x->x_fftsize * x->x_n_ls * sizeof(t_float));
	freebytes(x->x_beg_hrtf_re, x->x_n_ls * sizeof(t_float *));
	freebytes(x->x_beg_hrtf_im, x->x_n_ls * sizeof(t_float *));
}

/*
	1.arg: t_symbol *hrir_name;
	2.arg: t_symbol *hrtf_re_name;
	3.arg: t_symbol *hrtf_im_name;
	4.arg: t_symbol *hrir_fade_out_name;
	5.arg: int ambi_order;
	6.arg: int dim;
	7.arg: int n_ambi;
	8.arg: int fftsize;
	*/

static void *bin_ambi_calc_HRTF_new(t_symbol *s, int argc, t_atom *argv)
{
	t_bin_ambi_calc_HRTF *x = (t_bin_ambi_calc_HRTF *)pd_new(bin_ambi_calc_HRTF_class);
	char buf[400];
	int i, j, fftok;
	int n_ls, fftsize;
	t_symbol	*s_hrir;
	t_symbol	*s_hrtf_re;
	t_symbol	*s_hrtf_im;

	if((argc >= 6) &&
		IS_A_SYMBOL(argv,0) &&
		IS_A_SYMBOL(argv,1) &&
		IS_A_SYMBOL(argv,2) &&
		IS_A_SYMBOL(argv,3) &&
		IS_A_FLOAT(argv,4) &&
		IS_A_FLOAT(argv,5))
	{
		s_hrir								= (t_symbol *)atom_getsymbolarg(0, argc, argv);
		s_hrtf_re							= (t_symbol *)atom_getsymbolarg(1, argc, argv);
		s_hrtf_im							= (t_symbol *)atom_getsymbolarg(2, argc, argv);
		x->x_s_fade_out_hrir	= (t_symbol *)atom_getsymbolarg(3, argc, argv);

		n_ls		= (int)atom_getintarg(4, argc, argv);
		fftsize	= (int)atom_getintarg(5, argc, argv);

		if(n_ls < 1)
			n_ls = 1;

		j = 2;
		fftok = 0;
		for(i=0; i<21; i++)
		{
			if(j == fftsize)
			{
				fftok = 1;
				i = 22;
			}
			j *= 2;
		}

		if(!fftok)
		{
			post("bin_ambi_calc_HRTF-ERROR: fftsize not equal to 2 ^ n !!!");
			return(0);
		}

		x->x_n_ls			= n_ls;
		x->x_fftsize		= fftsize;

		x->x_hrir_filename	= (t_symbol **)getbytes(x->x_n_ls * sizeof(t_symbol *));
		x->x_s_hrir					= (t_symbol **)getbytes(x->x_n_ls * sizeof(t_symbol *));
		x->x_s_hrtf_re			= (t_symbol **)getbytes(x->x_n_ls * sizeof(t_symbol *));
		x->x_s_hrtf_im			= (t_symbol **)getbytes(x->x_n_ls * sizeof(t_symbol *));
		
		for(i=0; i<n_ls; i++)
		{
			sprintf(buf, "%d%s", i+1, s_hrir->s_name);
			x->x_s_hrir[i] = gensym(buf);

			sprintf(buf, "%d%s", i+1, s_hrtf_re->s_name);
			x->x_s_hrtf_re[i] = gensym(buf);

			sprintf(buf, "%d%s", i+1, s_hrtf_im->s_name);
			x->x_s_hrtf_im[i] = gensym(buf);
		}

		x->x_delta			= (int *)getbytes(x->x_n_ls * sizeof(int));
		x->x_phi				= (int *)getbytes(x->x_n_ls * sizeof(int));

		x->x_spec				= (BIN_AMBI_COMPLEX *)getbytes(x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));
		x->x_sin_cos		= (BIN_AMBI_COMPLEX *)getbytes(x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));

		x->x_beg_fade_out_hrir	= 0;
		x->x_beg_hrir						= (t_float *)getbytes(x->x_fftsize * x->x_n_ls * sizeof(t_float));
		x->x_beg_hrtf_re				= (iemarray_t **)getbytes(x->x_n_ls * sizeof(iemarray_t *));
		x->x_beg_hrtf_im				= (iemarray_t **)getbytes(x->x_n_ls * sizeof(iemarray_t *));

		x->x_pi_over_180	= 4.0 * atan(1.0) / 180.0;

		bin_ambi_calc_HRTF_init_cos(x);

		outlet_new(&x->x_obj, &s_list);
		return(x);
	}
	else
	{
		post("bin_ambi_calc_HRTF-ERROR: need 4 symbols + 2 floats arguments:");
		post("  hrir_name + hrtf_re_name + hrtf_im_name + hrir_fade_out_name +");
		post("  number_of_loudspeakers + fftsize");
		return(0);
	}
}

void bin_ambi_calc_HRTF_setup(void)
{
	bin_ambi_calc_HRTF_class = class_new(gensym("bin_ambi_calc_HRTF"), (t_newmethod)bin_ambi_calc_HRTF_new, (t_method)bin_ambi_calc_HRTF_free,
					 sizeof(t_bin_ambi_calc_HRTF), 0, A_GIMME, 0);
	class_addmethod(bin_ambi_calc_HRTF_class, (t_method)bin_ambi_calc_HRTF_ls, gensym("ls"), A_GIMME, 0);
	class_addmethod(bin_ambi_calc_HRTF_class, (t_method)bin_ambi_calc_HRTF_check_fade_out, gensym("check_fade_out"), 0);
	class_addmethod(bin_ambi_calc_HRTF_class, (t_method)bin_ambi_calc_HRTF_load_HRIR, gensym("load_HRIR"), A_FLOAT, 0);
	class_addmethod(bin_ambi_calc_HRTF_class, (t_method)bin_ambi_calc_HRTF_check_arrays, gensym("check_arrays"), A_FLOAT, 0);
	class_addmethod(bin_ambi_calc_HRTF_class, (t_method)bin_ambi_calc_HRTF_calc_fft, gensym("calc_fft"), A_FLOAT, 0);
//	class_sethelpsymbol(bin_ambi_calc_HRTF_class, gensym("iemhelp2/bin_ambi_calc_HRTF-help"));
}
/*
Reihenfolge:
n_ls x bin_ambi_calc_HRTF_ls

bin_ambi_calc_HRTF_check_fade_out

n_ls x bin_ambi_calc_HRTF_load_HRIR

n_ls x bin_ambi_calc_HRTF_check_arrays

n_ls x bin_ambi_calc_HRTF_calc_fft

*/
