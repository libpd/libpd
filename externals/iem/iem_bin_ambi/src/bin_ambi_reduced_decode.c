/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_bin_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2009 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_bin_ambi.h"
#include <math.h>
#include <stdio.h>
#include <string.h>


/* -------------------------- bin_ambi_reduced_decode ------------------------------ */
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
 ** setzt voraus , dass die HRIR-tabele-names von 1016_1_L_HRIR .. 1016_16_L_HRIR heissen und existieren **
 ** setzt voraus , dass die HRTF-tabele-names von 1016_1_HRTF_re .. 1016_16_HRTF_re heissen und existieren **
 ** setzt voraus , dass die HRTF-tabele-names von 1016_1_HRTF_im .. 1016_16_HRTF_im heissen und existieren **
 */

typedef struct _bin_ambi_reduced_decode
{
	t_object			x_obj;
	t_atom				x_at[2];
	int						x_n_dim;
	int						x_n_ambi;
	int						x_n_order;
	int						x_fftsize;
	double				*x_inv_work1;
	double				*x_inv_work2;
	double				*x_inv_buf2;
	double				*x_ambi_channel_weight;
	int						*x_delta;
	int						*x_phi;
	int						*x_phi_sym;
	int						*x_sym_flag;
	BIN_AMBI_COMPLEX	*x_spec;
	BIN_AMBI_COMPLEX	*x_sin_cos;
	iemarray_t					*x_beg_fade_out_hrir;
	t_float				*x_beg_hrir;
	iemarray_t			**x_beg_hrtf_re;
	iemarray_t			**x_beg_hrtf_im;
	t_symbol			**x_hrir_filename;
	t_symbol			**x_s_hrir;
	t_symbol			**x_s_hrtf_re;
	t_symbol			**x_s_hrtf_im;
	t_symbol			*x_s_fade_out_hrir;
	void					*x_out_sign_sum;
	double				x_sqrt3;
	double				x_sqrt10_4;
	double				x_sqrt15_2;
	double				x_sqrt6_4;
	double				x_sqrt35_8;
	double				x_sqrt70_4;
	double				x_sqrt5_2;
	double				x_sqrt126_16;
	double				x_sqrt315_8;
	double				x_sqrt105_4;
	double				x_pi_over_180;
	double				x_sing_range;
} t_bin_ambi_reduced_decode;

static t_class *bin_ambi_reduced_decode_class;

static void bin_ambi_reduced_decode_init_cos(t_bin_ambi_reduced_decode *x)
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

static void bin_ambi_reduced_decode_quant(t_bin_ambi_reduced_decode *x, double *delta_deg2rad, double *phi_deg2rad, int xindex)
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

	x->x_delta[xindex] = (int)(*delta_deg2rad);
	x->x_phi[xindex] = i;

	*delta_deg2rad *= x->x_pi_over_180;
	*phi_deg2rad *= x->x_pi_over_180;
}

static void bin_ambi_reduced_decode_do_2d(t_bin_ambi_reduced_decode *x, int argc, t_atom *argv)
{
	double delta=0.0, phi;
	double *dw = x->x_inv_work1;
	int xindex;
	int order=x->x_n_order;
	int n_ambi = x->x_n_ambi;

	if(argc < 2)
	{
		post("bin_ambi_reduced_decode ERROR: ls-input needs 1 index and 1 angle: ls_index + phi [degree]");
		return;
	}
	xindex = (int)atom_getint(argv++) - 1;
	phi = (double)atom_getfloat(argv);

	if(xindex < 0)
		xindex = 0;
	if(xindex >= n_ambi)
		xindex = n_ambi - 1;

	bin_ambi_reduced_decode_quant(x, &delta, &phi, xindex);

	dw += xindex;

	*dw = 1.0;
	dw += n_ambi;

	*dw = cos(phi);
	dw += n_ambi;
	*dw = sin(phi);
	dw += n_ambi;

	if(order >= 2)
	{
		*dw = cos(2.0*phi);
		dw += n_ambi;
		*dw = sin(2.0*phi);
		dw += n_ambi;

		if(order >= 3)
		{
			*dw = cos(3.0*phi);
			dw += n_ambi;
			*dw = sin(3.0*phi);
			dw += n_ambi;
			if(order >= 4)
			{
				*dw = cos(4.0*phi);
				dw += n_ambi;
				*dw = sin(4.0*phi);
				dw += n_ambi;

				if(order >= 5)
				{
					*dw = cos(5.0*phi);
					dw += n_ambi;
					*dw = sin(5.0*phi);
					dw += n_ambi;

					if(order >= 6)
					{
						*dw = cos(6.0*phi);
						dw += n_ambi;
						*dw = sin(6.0*phi);
						dw += n_ambi;

						if(order >= 7)
						{
							*dw = cos(7.0*phi);
							dw += n_ambi;
							*dw = sin(7.0*phi);
							dw += n_ambi;

							if(order >= 8)
							{
								*dw = cos(8.0*phi);
								dw += n_ambi;
								*dw = sin(8.0*phi);
								dw += n_ambi;

								if(order >= 9)
								{
									*dw = cos(9.0*phi);
									dw += n_ambi;
									*dw = sin(9.0*phi);
									dw += n_ambi;

									if(order >= 10)
									{
										*dw = cos(10.0*phi);
										dw += n_ambi;
										*dw = sin(10.0*phi);
										dw += n_ambi;

										if(order >= 11)
										{
											*dw = cos(11.0*phi);
											dw += n_ambi;
											*dw = sin(11.0*phi);
											dw += n_ambi;

											if(order >= 12)
											{
												*dw = cos(12.0*phi);
												dw += n_ambi;
												*dw = sin(12.0*phi);
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

static void bin_ambi_reduced_decode_do_3d(t_bin_ambi_reduced_decode *x, int argc, t_atom *argv)
{
	double delta, phi;
	double cd, sd, cd2, cd3, sd2, csd, cp, sp, cp2, sp2, cp3, sp3, cp4, sp4;
	double *dw = x->x_inv_work1;
	int xindex;
	int order=x->x_n_order;
	int n_ambi=x->x_n_ambi;

	if(argc < 3)
	{
		post("bin_ambi_reduced_decode ERROR: ls-input needs 1 index and 2 angles: ls index + delta [degree] + phi [degree]");
		return;
	}
	xindex = (int)atom_getint(argv++) - 1;
	delta = atom_getfloat(argv++);
	phi = atom_getfloat(argv);

	if(xindex < 0)
		xindex = 0;
	if(xindex >= n_ambi)
		xindex = n_ambi - 1;
	
	bin_ambi_reduced_decode_quant(x, &delta, &phi, xindex);

	dw += xindex;	

	cd = cos(delta);
	sd = sin(delta);
	cp = cos(phi);
	sp = sin(phi);
	
	*dw = 1.0;
	dw += n_ambi;

	*dw = cd * cp;
	dw += n_ambi;
	*dw = cd * sp;
	dw += n_ambi;
	*dw = sd;
	dw += n_ambi;

	if(order >= 2)
	{
		cp2 = cos(2.0*phi);
		sp2 = sin(2.0*phi);
		cd2 = cd * cd;
		sd2 = sd * sd;
		csd = cd * sd;
		*dw = 0.5 * x->x_sqrt3 * cd2 * cp2;
		dw += n_ambi;
		*dw = 0.5 * x->x_sqrt3 * cd2 * sp2;
		dw += n_ambi;
		*dw = x->x_sqrt3 * csd * cp;
		dw += n_ambi;
		*dw = x->x_sqrt3 * csd * sp;
		dw += n_ambi;
		*dw = 0.5 * (3.0 * sd2 - 1.0);
		dw += n_ambi;

		if(order >= 3)
		{
			cp3 = cos(3.0*phi);
			sp3 = sin(3.0*phi);
			cd3 = cd2 * cd;
			*dw = x->x_sqrt10_4 * cd3 * cp3;
			dw += n_ambi;
			*dw = x->x_sqrt10_4 * cd3 * sp3;
			dw += n_ambi;
			*dw = x->x_sqrt15_2 * cd * csd * cp2;
			dw += n_ambi;
			*dw = x->x_sqrt15_2 * cd * csd * sp2;
			dw += n_ambi;
			*dw = x->x_sqrt6_4 * cd * (5.0 * sd2 - 1.0) * cp;
			dw += n_ambi;
			*dw = x->x_sqrt6_4 * cd * (5.0 * sd2 - 1.0) * sp;
			dw += n_ambi;
			*dw = 0.5 * sd * (5.0 * sd2 - 3.0);
			dw += n_ambi;

			if(order >= 4)
			{
				cp4 = cos(4.0*phi);
				sp4 = sin(4.0*phi);
				*dw = x->x_sqrt35_8 * cd2 * cd2 * cp4;
				dw += n_ambi;
				*dw = x->x_sqrt35_8 * cd2 * cd2 * sp4;
				dw += n_ambi;
				*dw = x->x_sqrt70_4 * cd2 * csd * cp3;
				dw += n_ambi;
				*dw = x->x_sqrt70_4 * cd2 * csd * sp3;
				dw += n_ambi;
				*dw = 0.5 * x->x_sqrt5_2 * cd2 * (7.0 * sd2 - 1.0) * cp2;
				dw += n_ambi;
				*dw = 0.5 * x->x_sqrt5_2 * cd2 * (7.0 * sd2 - 1.0) * sp2;
				dw += n_ambi;
				*dw = x->x_sqrt10_4 * csd * (7.0 * sd2 - 3.0) * cp;
				dw += n_ambi;
				*dw = x->x_sqrt10_4 * csd * (7.0 * sd2 - 3.0) * sp;
				dw += n_ambi;
				*dw = 0.125 * (sd2 * (35.0 * sd2 - 30.0) + 3.0);
				dw += n_ambi;

				if(order >= 5)
				{
					*dw = x->x_sqrt126_16 * cd3 * cd2 * cos(5.0*phi);
					dw += n_ambi;
					*dw = x->x_sqrt126_16 * cd3 * cd2 * sin(5.0*phi);
					dw += n_ambi;
					*dw = x->x_sqrt315_8 * cd3 * csd * cp4;
					dw += n_ambi;
					*dw = x->x_sqrt315_8 * cd3 * csd * sp4;
					dw += n_ambi;
					*dw = 0.25 * x->x_sqrt70_4 * cd3 * (9.0 * sd2 - 1.0) * cp3;
					dw += n_ambi;
					*dw = 0.25 * x->x_sqrt70_4 * cd3 * (9.0 * sd2 - 1.0) * sp3;
					dw += n_ambi;
					*dw = x->x_sqrt105_4 * cd * csd * (3.0 * sd2 - 1.0) * cp2;
					dw += n_ambi;
					*dw = x->x_sqrt105_4 * cd * csd * (3.0 * sd2 - 1.0) * sp2;
					dw += n_ambi;
					*dw = 0.25 * x->x_sqrt15_2 * cd * (sd2 * (21.0 * sd2 - 14.0) + 1.0) * cp;
					dw += n_ambi;
					*dw = 0.25 * x->x_sqrt15_2 * cd * (sd2 * (21.0 * sd2 - 14.0) + 1.0) * sp;
					dw += n_ambi;
					*dw = 0.125 * sd * (sd2 * (63.0 * sd2 - 70.0) + 15.0);
				}
			}
		}
	}
}

static void bin_ambi_reduced_decode_ls(t_bin_ambi_reduced_decode *x, t_symbol *s, int argc, t_atom *argv)
{
	if(x->x_n_dim == 2)
		bin_ambi_reduced_decode_do_2d(x, argc, argv);
	else
		bin_ambi_reduced_decode_do_3d(x, argc, argv);
}

static void bin_ambi_reduced_decode_copy_row2buf(t_bin_ambi_reduced_decode *x, int row)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;
	double *db=x->x_inv_buf2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
		*db++ = *dw++;
}

static void bin_ambi_reduced_decode_copy_buf2row(t_bin_ambi_reduced_decode *x, int row)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;
	double *db=x->x_inv_buf2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
		*dw++ = *db++;
}

static void bin_ambi_reduced_decode_copy_row2row(t_bin_ambi_reduced_decode *x, int src_row, int dst_row)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw_src=x->x_inv_work2;
	double *dw_dst=x->x_inv_work2;

	dw_src += src_row*n_ambi2;
	dw_dst += dst_row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
		*dw_dst++ = *dw_src++;
}

static void bin_ambi_reduced_decode_xch_rows(t_bin_ambi_reduced_decode *x, int row1, int row2)
{
	bin_ambi_reduced_decode_copy_row2buf(x, row1);
	bin_ambi_reduced_decode_copy_row2row(x, row2, row1);
	bin_ambi_reduced_decode_copy_buf2row(x, row2);
}

static void bin_ambi_reduced_decode_mul_row(t_bin_ambi_reduced_decode *x, int row, double mul)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
	{
		(*dw) *= mul;
		dw++;
	}
}

static void bin_ambi_reduced_decode_mul_col(t_bin_ambi_reduced_decode *x, int col, double mul)
{
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	int i;
	double *dw=x->x_inv_work2;

	dw += col;
	for(i=0; i<n_ambi; i++)
	{
		(*dw) *= mul;
		dw += n_ambi2;
	}
}

static void bin_ambi_reduced_decode_mul_buf_and_add2row(t_bin_ambi_reduced_decode *x, int row, double mul)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;
	double *db=x->x_inv_buf2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
	{
		*dw += (*db)*mul;
		dw++;
		db++;
	}
}

static int bin_ambi_reduced_decode_eval_which_element_of_col_not_zero(t_bin_ambi_reduced_decode *x, int col, int start_row)
{
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	int i, j;
	double *dw=x->x_inv_work2;
	double singrange=x->x_sing_range;
	int ret=-1;

	dw += start_row*n_ambi2 + col;
	j = 0;
	for(i=start_row; i<n_ambi; i++)
	{
		if((*dw > singrange) || (*dw < -singrange))
		{
			ret = i;
			i = n_ambi+1;
		}
		dw += n_ambi2;
	}
	return(ret);
}

static void bin_ambi_reduced_decode_inverse(t_bin_ambi_reduced_decode *x)
{
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	int i, j, nz;
	int r,c;
	double *src=x->x_inv_work1;
	double *db=x->x_inv_work2;
	double *acw_vec=x->x_ambi_channel_weight;
	double rcp, *dv;

	dv = db;
	for(i=0; i<n_ambi; i++) /* init */
	{
		for(j=0; j<n_ambi; j++)
		{
			*dv++ = *src++;
		}
		for(j=0; j<n_ambi; j++)
		{
			if(j == i)
				*dv++ = 1.0;
			else
				*dv++ = 0.0;
		}
	}
/* make 1 in main-diagonale, and 0 below */
	for(i=0; i<n_ambi; i++)
	{
		nz = bin_ambi_reduced_decode_eval_which_element_of_col_not_zero(x, i, i);
		if(nz < 0)
		{
			post("bin_ambi_reduced_decode ERROR: matrix singular !!!!");
			return;
		}
		else
		{
			if(nz != i)
				bin_ambi_reduced_decode_xch_rows(x, i, nz);
			dv = db + i*n_ambi2 + i;
			rcp = 1.0 /(*dv);
			bin_ambi_reduced_decode_mul_row(x, i, rcp);
			bin_ambi_reduced_decode_copy_row2buf(x, i);
			for(j=i+1; j<n_ambi; j++)
			{
				dv += n_ambi2;
				rcp = -(*dv);
				bin_ambi_reduced_decode_mul_buf_and_add2row(x, j, rcp);
			}
		}
	}
/* make 0 above the main diagonale */
	for(i=n_ambi-1; i>=0; i--)
	{
		dv = db + i*n_ambi2 + i;
		bin_ambi_reduced_decode_copy_row2buf(x, i);
		for(j=i-1; j>=0; j--)
		{
			dv -= n_ambi2;
			rcp = -(*dv);
			bin_ambi_reduced_decode_mul_buf_and_add2row(x, j, rcp);
		}
	}

	for(i=0; i<n_ambi; i++)
	{
		bin_ambi_reduced_decode_mul_col(x, i+n_ambi, acw_vec[i]);
	}
	
	post("matrix_inverse regular");
}

static void bin_ambi_reduced_decode_calc_pinv(t_bin_ambi_reduced_decode *x)
{
	t_garray *a;
	int npoints;
	iemarray_t *fadevec;

	if(x->x_beg_fade_out_hrir == 0)
	{
		if (!(a = (t_garray *)pd_findbyclass(x->x_s_fade_out_hrir, garray_class)))
			error("%s: no such array", x->x_s_fade_out_hrir->s_name);
		else if (!iemarray_getarray(a, &npoints, &fadevec))
			error("%s: bad template for bin_ambi_reduced_decode", x->x_s_fade_out_hrir->s_name);
		else if (npoints < x->x_fftsize)
			error("%s: bad array-size: %d", x->x_s_fade_out_hrir->s_name, npoints);
		else
			x->x_beg_fade_out_hrir = fadevec;
	}

	bin_ambi_reduced_decode_inverse(x);
}

/*
x_prod:
n_ambi  columns;
n_ambi    rows;
*/

static void bin_ambi_reduced_decode_load_HRIR(t_bin_ambi_reduced_decode *x, t_float findex)
{
	int xindex=(int)findex - 1;
	int p;
	char buf[60];

	if(xindex < 0)
		xindex = 0;
	if(xindex >= x->x_n_ambi)
		xindex = x->x_n_ambi - 1;

	p = x->x_phi[xindex];

	if(p)/*change*/
		p = 360 - p;

	if(p < 10)
		sprintf(buf, "L%de00%da.wav", x->x_delta[xindex], p);
	else if(p < 100)
		sprintf(buf, "L%de0%da.wav", x->x_delta[xindex], p);
	else
		sprintf(buf, "L%de%da.wav", x->x_delta[xindex], p);
	x->x_hrir_filename[xindex] = gensym(buf);

	SETSYMBOL(x->x_at, x->x_hrir_filename[xindex]);
	SETSYMBOL(x->x_at+1, x->x_s_hrir[xindex]);
	outlet_list(x->x_obj.ob_outlet, &s_list, 2, x->x_at);
}

static void bin_ambi_reduced_decode_check_arrays(t_bin_ambi_reduced_decode *x, t_float findex)
{
	int xindex=(int)findex - 1;
	int j, k, n;
	int fftsize = x->x_fftsize;
	int fs2=fftsize/2;
	t_garray *a;
	int npoints;
	iemarray_t *vec_hrir;
	t_float *vec;
	iemarray_t *vec_fade_out_hrir;
	iemarray_t *vec_hrtf_re, *vec_hrtf_im;
	t_symbol *hrir, *hrtf_re, *hrtf_im;
	t_float decr, sum;

	if(xindex < 0)
		xindex = 0;
	if(xindex >= x->x_n_ambi)
		xindex = x->x_n_ambi - 1;

	hrir = x->x_s_hrir[xindex];
	hrtf_re = x->x_s_hrtf_re[xindex];
	hrtf_im = x->x_s_hrtf_im[xindex];

	if (!(a = (t_garray *)pd_findbyclass(hrtf_re, garray_class)))
		error("%s: no such array", hrtf_re->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrtf_re))
		error("%s: bad template for bin_ambi_reduced_decode", hrtf_re->s_name);
	else if (npoints < fftsize)
		error("%s: bad array-size: %d", hrtf_re->s_name, npoints);
	else if (!(a = (t_garray *)pd_findbyclass(hrtf_im, garray_class)))
		error("%s: no such array", hrtf_im->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrtf_im))
		error("%s: bad template for bin_ambi_reduced_decode", hrtf_im->s_name);
	else if (npoints < fftsize)
		error("%s: bad array-size: %d", hrtf_im->s_name, npoints);
	else if (!(a = (t_garray *)pd_findbyclass(hrir, garray_class)))
		error("%s: no such array", hrir->s_name);
	else if (!iemarray_getarray(a, &npoints, &vec_hrir))
		error("%s: bad template for bin_ambi_reduced_decode", hrir->s_name);
	else
	{
		x->x_beg_hrtf_re[xindex] = vec_hrtf_re;
		x->x_beg_hrtf_im[xindex] = vec_hrtf_im;

		if(npoints < fftsize)
		{
			post("bin_ambi_reduced_decode-WARNING: %s-array-size: %d < FFT-size: %d", hrir->s_name, npoints, fftsize);
		}
		vec = x->x_beg_hrir;
		vec += xindex * fftsize;
	
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

static void bin_ambi_reduced_decode_calc_reduced(t_bin_ambi_reduced_decode *x, t_float findex)
{
	int xindex=(int)findex - 1;
	int i, j, k, w_index, w_inc, i_inc, v_index, fs1, fs2;
	int fftsize = x->x_fftsize;
	BIN_AMBI_COMPLEX old1, old2, w;
	BIN_AMBI_COMPLEX *sincos = x->x_sin_cos;
	BIN_AMBI_COMPLEX *val = x->x_spec;
	t_float *vec_hrir;
	iemarray_t *vec_hrtf_re, *vec_hrtf_im;
	double *dv, *db=x->x_inv_work2;
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	t_float mul;

	if(xindex < 0)
		xindex = 0;
	if(xindex >= n_ambi)
		xindex = n_ambi - 1;

	vec_hrtf_re = x->x_beg_hrtf_re[xindex];
	vec_hrtf_im = x->x_beg_hrtf_im[xindex];

	dv = db + xindex + n_ambi;
	mul = (t_float)(*dv);
	vec_hrir = x->x_beg_hrir;
	for(k=0; k<fftsize; k++)
	{
		val[k].real = mul * vec_hrir[k];
		val[k].imag = 0.0f;
	}

	for(j=1; j<n_ambi; j++)
	{
		dv += n_ambi2;
		mul = (t_float)(*dv);
		vec_hrir = x->x_beg_hrir;
		vec_hrir += j * fftsize;
		for(k=0; k<fftsize; k++)
		{
			val[k].real += mul * vec_hrir[k];
		}
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
    iemarray_setfloat(vec_hrtf_re, i, val[i].real);
    iemarray_setfloat(vec_hrtf_im, i, val[i].imag);
  }
  iemarray_setfloat(vec_hrtf_re, fs2, val[fs2].real);
  iemarray_setfloat(vec_hrtf_im, fs2, 0.0f);
  for(i = fs2+1; i < fftsize; i++)
  {
    iemarray_setfloat(vec_hrtf_re, i, 0.0f);
    iemarray_setfloat(vec_hrtf_im, i, 0.0f);
  }
}

static void bin_ambi_reduced_decode_decoding(t_bin_ambi_reduced_decode *x)
{
	int *sym=x->x_phi_sym;
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	int *phi=x->x_phi;
	int *delta=x->x_delta;
	int *flag=x->x_sym_flag;
	int i, j, d, p, regular=1;
	double *dv, *db=x->x_inv_work2;
	double a, b, q;
	int sym_max=0;
	int pos_sym_counter, neg_sym_counter;
	char plus_minus[100];
	char abc[100];
	char onenine[100];
	char ten[100];

	for(i=0; i<n_ambi; i++)
		sym[i] = -1;

	plus_minus[0] = 0;
	if(x->x_n_dim == 2)
	{
		strcpy(abc,     "                      WXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXYXY");
		strcpy(ten,     "                      000000000000000000011111111111111111111");
		strcpy(onenine, "                      011223344556677889900112233445566778899");
		abc[n_ambi+22] = 0;
		ten[n_ambi+22] = 0;
		onenine[n_ambi+22] = 0;
		post(abc);
		post(ten);
		post(onenine);
	}
	else
	{
		strcpy(abc,     "                      AABCABCDEABCDEFGABCDEFGHIABCDEFGHIJKABCDEFGHIJKLM");
		strcpy(onenine, "                      0111222223333333444444444555555555556666666666666");
		abc[n_ambi+22] = 0;
		onenine[n_ambi+22] = 0;
		post(abc);
		post(onenine);
	}

	for(i=0; i<n_ambi; i++)
	{
		d = delta[i];
		p = phi[i];
		for(j=i+1; j<n_ambi; j++)
		{
			if(d == delta[j])
			{
				if((p + phi[j]) == 360)
				{
					if((sym[i] < 0) && (sym[j] < 0))
					{
						sym[i] = j;
						sym[j] = i;
						j = n_ambi + 1;
						sym_max++;
					}
				}
			}
		}
	}

	for(p=0; p<n_ambi; p++)/*each col*/
	{
		pos_sym_counter = 0;
		neg_sym_counter = 0;
		for(i=0; i<n_ambi; i++)
			flag[i] = 1;
		dv = db + p + n_ambi;
		for(i=0; i<n_ambi; i++)
		{
			if((sym[i] >= 0) && flag[i])
			{
				j = sym[i];
				flag[i] = 0;
				flag[j] = 0;
				a = dv[i*n_ambi2];
				b = dv[j*n_ambi2];
				if((a < 5.0e-4)&&(a > -5.0e-4)&&(b < 5.0e-4)&&(b > -5.0e-4))
				{
					pos_sym_counter++;
					neg_sym_counter++;
				}
				else
				{
					q = a / b;
					if((q < 1.005)&&(q > 0.995))
						pos_sym_counter++;
					else if((q > -1.005)&&(q < -0.995))
						neg_sym_counter++;
				}
			}
		}
		if(pos_sym_counter == sym_max)
			strcat(plus_minus, "+");
		else if(neg_sym_counter == sym_max)
			strcat(plus_minus, "-");
		else
		{
			strcat(plus_minus, "?");
			regular = 0;
		}
	}
	post("sum of right channel: %s", plus_minus);

	if(regular)
	{
		for(i=0; i<n_ambi; i++)
		{
			/*change*/
			if(plus_minus[i] == '+')
				SETFLOAT(x->x_at, 1.0f);
			else if(plus_minus[i] == '-')
				SETFLOAT(x->x_at, 2.0f);
			SETFLOAT(x->x_at+1, (t_float)(i+1));
			outlet_list(x->x_out_sign_sum, &s_list, 2, x->x_at);
		}
	}
}

static void bin_ambi_reduced_decode_ambi_weight(t_bin_ambi_reduced_decode *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc > x->x_n_order)
	{
		int i, k=0, n=x->x_n_order;
		double d;

		x->x_ambi_channel_weight[k] = (double)atom_getfloat(argv++);
		k++;
		if(x->x_n_dim == 2)
		{
			for(i=1; i<=n; i++)
			{
				d = (double)atom_getfloat(argv++);
				x->x_ambi_channel_weight[k] = d;
				k++;
				x->x_ambi_channel_weight[k] = d;
				k++;
			}
		}
		else
		{
			int j, m;

			for(i=1; i<=n; i++)
			{
				d = (double)atom_getfloat(argv++);
				m = 2*i + 1;
				for(j=0; j<m; j++)
				{
					x->x_ambi_channel_weight[k] = d;
					k++;
				}
			}
		}
	}
	else
		post("bin_ambi_reduced_decode-ERROR: ambi_weight needs %d float weights", x->x_n_order+1);
}

static void bin_ambi_reduced_decode_sing_range(t_bin_ambi_reduced_decode *x, t_floatarg f)
{
	if(f < 0.0f)
		x->x_sing_range = -(double)f;
	else
		x->x_sing_range = (double)f;
}

static void bin_ambi_reduced_decode_free(t_bin_ambi_reduced_decode *x)
{
	freebytes(x->x_hrir_filename, x->x_n_ambi * sizeof(t_symbol *));
	freebytes(x->x_s_hrir, x->x_n_ambi * sizeof(t_symbol *));
	freebytes(x->x_s_hrtf_re, x->x_n_ambi * sizeof(t_symbol *));
	freebytes(x->x_s_hrtf_im, x->x_n_ambi * sizeof(t_symbol *));
	freebytes(x->x_inv_work1, x->x_n_ambi * x->x_n_ambi * sizeof(double));
	freebytes(x->x_inv_work2, 2 * x->x_n_ambi * x->x_n_ambi * sizeof(double));
	freebytes(x->x_inv_buf2, 2 * x->x_n_ambi * sizeof(double));
	freebytes(x->x_ambi_channel_weight, x->x_n_ambi * sizeof(double));

	freebytes(x->x_delta, x->x_n_ambi * sizeof(int));
	freebytes(x->x_phi, x->x_n_ambi * sizeof(int));
	freebytes(x->x_phi_sym, x->x_n_ambi * sizeof(int));
	freebytes(x->x_sym_flag, x->x_n_ambi * sizeof(int));

	freebytes(x->x_spec, x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));
	freebytes(x->x_sin_cos, x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));

	freebytes(x->x_beg_hrir, x->x_fftsize * x->x_n_ambi * sizeof(t_float));
	freebytes(x->x_beg_hrtf_re, x->x_n_ambi * sizeof(iemarray_t *));
	freebytes(x->x_beg_hrtf_im, x->x_n_ambi * sizeof(iemarray_t *));
}

/*
  1.arg_ int prefix;
	2.arg: t_symbol *hrir_name;
	3.arg: t_symbol *hrtf_re_name;
	4.arg: t_symbol *hrtf_im_name;
	5.arg: t_symbol *hrir_fade_out_name;
	6.arg: int ambi_order;
	7.arg: int dim;
	8.arg: int fftsize;
	*/

static void *bin_ambi_reduced_decode_new(t_symbol *s, int argc, t_atom *argv)
{
	t_bin_ambi_reduced_decode *x = (t_bin_ambi_reduced_decode *)pd_new(bin_ambi_reduced_decode_class);
	char buf[400];
	int i, j, fftok, ok=0;
	int n_order=0, n_dim=0, n_ambi=0, fftsize=0, prefix=0;
	t_symbol	*s_hrir=gensym("L_HRIR");
	t_symbol	*s_hrtf_re=gensym("HRTF_re");
	t_symbol	*s_hrtf_im=gensym("HRTF_im");
  t_symbol  *s_fade_out_hrir=gensym("HRIR_win");

	if((argc >= 8) &&
		IS_A_FLOAT(argv,0) &&
    IS_A_SYMBOL(argv,1) &&
		IS_A_SYMBOL(argv,2) &&
		IS_A_SYMBOL(argv,3) &&
		IS_A_SYMBOL(argv,4) &&
		IS_A_FLOAT(argv,5) &&
		IS_A_FLOAT(argv,6) &&
		IS_A_FLOAT(argv,7))
	{
		prefix	= (int)atom_getintarg(0, argc, argv);

		s_hrir								= (t_symbol *)atom_getsymbolarg(1, argc, argv);
		s_hrtf_re							= (t_symbol *)atom_getsymbolarg(2, argc, argv);
		s_hrtf_im							= (t_symbol *)atom_getsymbolarg(3, argc, argv);
		s_fade_out_hrir	      = (t_symbol *)atom_getsymbolarg(4, argc, argv);

		n_order	= (int)atom_getintarg(5, argc, argv);
		n_dim		= (int)atom_getintarg(6, argc, argv);
		fftsize	= (int)atom_getintarg(7, argc, argv);

		ok = 1;
	}
	else if((argc >= 8) &&
		IS_A_FLOAT(argv,0) &&
    IS_A_FLOAT(argv,1) &&
		IS_A_FLOAT(argv,2) &&
		IS_A_FLOAT(argv,3) &&
		IS_A_FLOAT(argv,4) &&
		IS_A_FLOAT(argv,5) &&
		IS_A_FLOAT(argv,6) &&
		IS_A_FLOAT(argv,7))
	{
		prefix	= (int)atom_getintarg(0, argc, argv);

		s_hrir								= gensym("L_HRIR");
		s_hrtf_re							= gensym("HRTF_re");
		s_hrtf_im							= gensym("HRTF_im");
		s_fade_out_hrir	      = gensym("HRIR_win");

		n_order	= (int)atom_getintarg(5, argc, argv);
		n_dim		= (int)atom_getintarg(6, argc, argv);
		fftsize	= (int)atom_getintarg(7, argc, argv);

		ok = 1;
	}

	if(ok)
	{
		if(n_order < 1)
			n_order = 1;

		if(n_dim == 3)
		{
			if(n_order > 5)
				n_order = 5;
			n_ambi = (n_order + 1)*(n_order + 1);
		}
		else
		{
			if(n_order > 12)
				n_order = 12;
			n_dim = 2;
			n_ambi = 2 * n_order + 1;
		}

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
      fftsize = 512;
			post("bin_ambi_reduced_decode-WARNING: fftsize not equal to 2 ^ n !!!");
			post("                                 fftsize set to %d", fftsize);
		}

		x->x_n_dim			= n_dim;
		x->x_n_ambi			= n_ambi;
		x->x_n_order		= n_order;
		x->x_fftsize		= fftsize;

		x->x_hrir_filename	= (t_symbol **)getbytes(x->x_n_ambi * sizeof(t_symbol *));
		x->x_s_hrir					= (t_symbol **)getbytes(x->x_n_ambi * sizeof(t_symbol *));
		x->x_s_hrtf_re			= (t_symbol **)getbytes(x->x_n_ambi * sizeof(t_symbol *));
		x->x_s_hrtf_im			= (t_symbol **)getbytes(x->x_n_ambi * sizeof(t_symbol *));
		
		for(i=0; i<n_ambi; i++)
		{
			sprintf(buf, "%d_%d_%s", prefix, i+1, s_hrir->s_name);
			x->x_s_hrir[i] = gensym(buf);

			sprintf(buf, "%d_%d_%s", prefix, i+1, s_hrtf_re->s_name);
			x->x_s_hrtf_re[i] = gensym(buf);

			sprintf(buf, "%d_%d_%s", prefix, i+1, s_hrtf_im->s_name);
			x->x_s_hrtf_im[i] = gensym(buf);
		}

    sprintf(buf, "%d_%s", prefix, s_fade_out_hrir->s_name);
    x->x_s_fade_out_hrir = gensym(buf);

		x->x_inv_work1	= (double *)getbytes(x->x_n_ambi * x->x_n_ambi * sizeof(double));
		x->x_inv_work2	= (double *)getbytes(2 * x->x_n_ambi * x->x_n_ambi * sizeof(double));
		x->x_inv_buf2		= (double *)getbytes(2 * x->x_n_ambi * sizeof(double));
		x->x_ambi_channel_weight = (double *)getbytes(x->x_n_ambi * sizeof(double));

		x->x_delta			= (int *)getbytes(x->x_n_ambi * sizeof(int));
		x->x_phi				= (int *)getbytes(x->x_n_ambi * sizeof(int));
		x->x_phi_sym		= (int *)getbytes(x->x_n_ambi * sizeof(int));
		x->x_sym_flag		= (int *)getbytes(x->x_n_ambi * sizeof(int));

		x->x_spec				= (BIN_AMBI_COMPLEX *)getbytes(x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));
		x->x_sin_cos		= (BIN_AMBI_COMPLEX *)getbytes(x->x_fftsize * sizeof(BIN_AMBI_COMPLEX));

		x->x_beg_fade_out_hrir	= 0;
		x->x_beg_hrir						= (t_float *)getbytes(x->x_fftsize * x->x_n_ambi * sizeof(t_float));
		x->x_beg_hrtf_re				= (iemarray_t **)getbytes(x->x_n_ambi * sizeof(iemarray_t *));
		x->x_beg_hrtf_im				= (iemarray_t **)getbytes(x->x_n_ambi * sizeof(iemarray_t *));

		x->x_sqrt3				= sqrt(3.0);
		x->x_sqrt5_2			= sqrt(5.0) / 2.0;
		x->x_sqrt6_4			= sqrt(6.0) / 4.0;
		x->x_sqrt10_4			= sqrt(10.0) / 4.0;
		x->x_sqrt15_2			= sqrt(15.0) / 2.0;
		x->x_sqrt35_8			= sqrt(35.0) / 8.0;
		x->x_sqrt70_4			= sqrt(70.0) / 4.0;
		x->x_sqrt126_16		= sqrt(126.0) / 16.0;
		x->x_sqrt315_8		= sqrt(315.0) / 8.0;
		x->x_sqrt105_4		= sqrt(105.0) / 4.0;
		x->x_pi_over_180	= 4.0 * atan(1.0) / 180.0;
		x->x_sing_range = 1.0e-10;
		for(i=0; i<n_ambi; i++)
			x->x_ambi_channel_weight[i] = 1.0;

		bin_ambi_reduced_decode_init_cos(x);

		outlet_new(&x->x_obj, &s_list);
		x->x_out_sign_sum = outlet_new(&x->x_obj, &s_list);
		return(x);
	}
	else
	{
		post("bin_ambi_reduced_decode-ERROR: need 1 float + 4 symbols + 3 floats arguments:");
		post("  prefix(unique-number) + hrir_name + hrtf_re_name + hrtf_im_name + hrir_fade_out_name +");
		post("   + ambi_order + ambi_dimension + fftsize");
		return(0);
	}
}

void bin_ambi_reduced_decode_setup(void)
{
	bin_ambi_reduced_decode_class = class_new(gensym("bin_ambi_reduced_decode"), (t_newmethod)bin_ambi_reduced_decode_new, (t_method)bin_ambi_reduced_decode_free,
					 sizeof(t_bin_ambi_reduced_decode), 0, A_GIMME, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_ls, gensym("ls"), A_GIMME, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_calc_pinv, gensym("calc_pinv"), 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_load_HRIR, gensym("load_HRIR"), A_FLOAT, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_check_arrays, gensym("check_arrays"), A_FLOAT, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_calc_reduced, gensym("calc_reduced"), A_FLOAT, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_decoding, gensym("decoding"), 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_ambi_weight, gensym("ambi_weight"), A_GIMME, 0);
	class_addmethod(bin_ambi_reduced_decode_class, (t_method)bin_ambi_reduced_decode_sing_range, gensym("sing_range"), A_DEFFLOAT, 0);
//	class_sethelpsymbol(bin_ambi_reduced_decode_class, gensym("iemhelp2/bin_ambi_reduced_decode-help"));
}
/*
Reihenfolge:
n_ambi x bin_ambi_reduced_decode_ls

bin_ambi_reduced_decode_calc_pinv

n_ambi x bin_ambi_reduced_decode_load_HRIR

n_ambi x bin_ambi_reduced_decode_check_arrays

n_ambi x bin_ambi_reduced_decode_calc_reduced

bin_ambi_reduced_decode_decoding

*/
