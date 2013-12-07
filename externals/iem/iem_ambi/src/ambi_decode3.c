/* For information on usage and redistribution, and for a DISCLAIMER OF ALL
* WARRANTIES, see the file, "LICENSE.txt," in this distribution.

iem_ambi written by Thomas Musil, Copyright (c) IEM KUG Graz Austria 2000 - 2006 */

#include "m_pd.h"
#include "iemlib.h"
#include "iem_ambi.h"
#include <math.h>


/* -------------------------- ambi_decode3 ------------------------------ */
/*
 
 */

typedef struct _ambi_decode3
{
	t_object	x_obj;
	t_atom		*x_at;
	double		*x_inv_work1;
	double		*x_inv_work2;
	double		*x_inv_buf2;
	double		*x_transp;
	double		*x_ls_encode;
	double		*x_prod;
	double		*x_ambi_channel_weight;
	double		x_sing_range;
	int				x_n_ambi;
	int				x_n_order;
	int				x_n_real_ls;
	int				x_n_pht_ls;
	int				x_n_dim;
	t_symbol	*x_s_matrix;
	double		x_sqrt3;
	double		x_sqrt10_4;
	double		x_sqrt15_2;
	double		x_sqrt6_4;
	double		x_sqrt35_8;
	double		x_sqrt70_4;
	double		x_sqrt5_2;
	double		x_sqrt126_16;
	double		x_sqrt315_8;
	double		x_sqrt105_4;
	double		x_pi_over_180;
} t_ambi_decode3;

static t_class *ambi_decode3_class;

static void ambi_decode3_copy_row2buf(t_ambi_decode3 *x, int row)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;
	double *db=x->x_inv_buf2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
		*db++ = *dw++;
}

static void ambi_decode3_copy_buf2row(t_ambi_decode3 *x, int row)
{
	int n_ambi2 = 2*x->x_n_ambi;
	int i;
	double *dw=x->x_inv_work2;
	double *db=x->x_inv_buf2;

	dw += row*n_ambi2;
	for(i=0; i<n_ambi2; i++)
		*dw++ = *db++;
}

static void ambi_decode3_copy_row2row(t_ambi_decode3 *x, int src_row, int dst_row)
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

static void ambi_decode3_xch_rows(t_ambi_decode3 *x, int row1, int row2)
{
	ambi_decode3_copy_row2buf(x, row1);
	ambi_decode3_copy_row2row(x, row2, row1);
	ambi_decode3_copy_buf2row(x, row2);
}

static void ambi_decode3_mul_row(t_ambi_decode3 *x, int row, double mul)
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

static void ambi_decode3_mul_buf_and_add2row(t_ambi_decode3 *x, int row, double mul)
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

static int ambi_decode3_eval_which_element_of_col_not_zero(t_ambi_decode3 *x, int col, int start_row)
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

static void ambi_decode3_mul1(t_ambi_decode3 *x)
{
	double *vec1, *beg1=x->x_ls_encode;
	double *vec2, *beg2=x->x_ls_encode;
	double *inv=x->x_inv_work1;
	double sum;
	int n_ls=x->x_n_real_ls+x->x_n_pht_ls;
	int n_ambi=x->x_n_ambi;
	int i, j, k;

	for(k=0; k<n_ambi; k++)
	{
		beg2=x->x_ls_encode;
		for(j=0; j<n_ambi; j++)
		{
			sum = 0.0;
			vec1 = beg1;
			vec2 = beg2;
			for(i=0; i<n_ls; i++)
			{
				sum += *vec1++ * *vec2++;
			}
			beg2 += n_ls;
			*inv++ = sum;
		}
		beg1 += n_ls;
	}
}

static void ambi_decode3_mul2(t_ambi_decode3 *x)
{
	int n_ls=x->x_n_real_ls+x->x_n_pht_ls;
	int n_ambi=x->x_n_ambi;
	int n_ambi2=2*n_ambi;
	int i, j, k;
	double *vec1, *beg1=x->x_transp;
	double *vec2, *beg2=x->x_inv_work2+n_ambi;
	double *vec3=x->x_prod;
	double *acw_vec=x->x_ambi_channel_weight;
	double sum;

	for(k=0; k<n_ls; k++)
	{
		beg2=x->x_inv_work2+n_ambi;
		for(j=0; j<n_ambi; j++)
		{
			sum = 0.0;
			vec1 = beg1;
			vec2 = beg2;
			for(i=0; i<n_ambi; i++)
			{
				sum += *vec1++ * *vec2;
				vec2 += n_ambi2;
			}
			beg2++;
			*vec3++ = sum * acw_vec[j];
		}
		beg1 += n_ambi;
	}
}

static void ambi_decode3_transp_back(t_ambi_decode3 *x)
{
	double *vec, *transp=x->x_transp;
	double *straight=x->x_ls_encode;
	int n_ls=x->x_n_real_ls+x->x_n_pht_ls;
	int n_ambi=x->x_n_ambi;
	int i, j;

	for(j=0; j<n_ambi; j++)
	{
		vec = transp;
		for(i=0; i<n_ls; i++)
		{
			*straight++ = *vec;
			vec += n_ambi;
		}
		transp++;
	}
}

static void ambi_decode3_inverse(t_ambi_decode3 *x)
{
	int n_ambi = x->x_n_ambi;
	int n_ambi2 = 2*n_ambi;
	int i, j, nz;
	int r,c;
	double *src=x->x_inv_work1;
	double *db=x->x_inv_work2;
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
		nz = ambi_decode3_eval_which_element_of_col_not_zero(x, i, i);
		if(nz < 0)
		{
			post("ambi_decode3 ERROR: matrix singular !!!!");
			return;
		}
		else
		{
			if(nz != i)
				ambi_decode3_xch_rows(x, i, nz);
			dv = db + i*n_ambi2 + i;
			rcp = 1.0 /(*dv);
			ambi_decode3_mul_row(x, i, rcp);
			ambi_decode3_copy_row2buf(x, i);
			for(j=i+1; j<n_ambi; j++)
			{
				dv += n_ambi2;
				rcp = -(*dv);
				ambi_decode3_mul_buf_and_add2row(x, j, rcp);
			}
		}
	}

			/* make 0 above the main diagonale */
	for(i=n_ambi-1; i>=0; i--)
	{
		dv = db + i*n_ambi2 + i;
		ambi_decode3_copy_row2buf(x, i);
		for(j=i-1; j>=0; j--)
		{
			dv -= n_ambi2;
			rcp = -(*dv);
			ambi_decode3_mul_buf_and_add2row(x, j, rcp);
		}
	}

	post("matrix_inverse nonsingular");
}

static void ambi_decode3_begin_pseudo_inverse(t_ambi_decode3 *x)
{
	t_atom *at=x->x_at;
	int i, n=x->x_n_real_ls*x->x_n_ambi;
	double *dv1=x->x_prod;

	ambi_decode3_transp_back(x);
	ambi_decode3_mul1(x);
	ambi_decode3_inverse(x);
	ambi_decode3_mul2(x);
	at += 2;
	for(i=0; i<n; i++)
	{
		SETFLOAT(at, (t_float)(*dv1));
		dv1++;
		at++;
	}
}

static void ambi_decode3_ipht_ireal_muladd(t_ambi_decode3 *x, t_symbol *s, int argc, t_atom *argv)
{
	t_atom *at=x->x_at;
	int i, n=x->x_n_ambi;
	int pht_index, real_index;
	double mw;
	t_float dat1;
	double *dv2=x->x_prod;

	if(argc < 3)
	{
		post("ambi_decode3 ERROR: ipht_ireal_muladd needs 2 index and 1 mirrorweight: pht_ls_index + real_ls_index + mirror_weight_element");
		return;
	}
	pht_index = (int)atom_getint(argv++) - 1;
	real_index = (int)atom_getint(argv++) - 1;
	mw = (double)atom_getfloat(argv);

	if(pht_index < 0)
		pht_index = 0;
	if(real_index < 0)
		real_index = 0;
	if(real_index >= x->x_n_real_ls)
		real_index = x->x_n_real_ls - 1;
	if(pht_index >= x->x_n_pht_ls)
		pht_index = x->x_n_pht_ls - 1;

	at += 2 + (real_index)*x->x_n_ambi;
	dv2 += (x->x_n_real_ls+pht_index)*x->x_n_ambi;
	for(i=0; i<n; i++)
	{
		dat1 = atom_getfloat(at);
		SETFLOAT(at, dat1 + (t_float)(*dv2*mw));
		dv2++;
		at++;
	}
}

static void ambi_decode3_end_pseudo_inverse(t_ambi_decode3 *x)
{
	outlet_anything(x->x_obj.ob_outlet, x->x_s_matrix, x->x_n_ambi*x->x_n_real_ls+2, x->x_at);
}

static void ambi_decode3_encode_ls_2d(t_ambi_decode3 *x, int argc, t_atom *argv, int mode)
{
	double phi;
	double *dw = x->x_transp;
	int index;
	int order=x->x_n_order;

	if(argc < 2)
	{
		post("ambi_decode3 ERROR: ls-input needs 1 index and 1 angle: ls_index + phi [degree]");
		return;
	}
	index = (int)atom_getint(argv++) - 1;
	phi = (double)atom_getfloat(argv);

	if(index < 0)
		index = 0;

	if(mode == AMBI_LS_REAL)
	{
		if(index >= x->x_n_real_ls)
			index = x->x_n_real_ls - 1;
	}
	else if(mode == AMBI_LS_PHT)
	{
		if(x->x_n_pht_ls)
		{
			if(index >= x->x_n_pht_ls)
				index = x->x_n_pht_ls - 1;
			index += x->x_n_real_ls;
		}
		else
			return;
	}
	else
		return;
	
	phi *= x->x_pi_over_180;

	dw += index * x->x_n_ambi;

	*dw++ = 1.0;
	*dw++ = cos(phi);
	*dw++ = sin(phi);

	if(order >= 2)
	{
		*dw++ = cos(2.0*phi);
		*dw++ = sin(2.0*phi);

		if(order >= 3)
		{
			*dw++ = cos(3.0*phi);
			*dw++ = sin(3.0*phi);
			if(order >= 4)
			{
				*dw++ = cos(4.0*phi);
				*dw++ = sin(4.0*phi);

				if(order >= 5)
				{
					*dw++ = cos(5.0*phi);
					*dw++ = sin(5.0*phi);

					if(order >= 6)
					{
						*dw++ = cos(6.0*phi);
						*dw++ = sin(6.0*phi);

						if(order >= 7)
						{
							*dw++ = cos(7.0*phi);
							*dw++ = sin(7.0*phi);

							if(order >= 8)
							{
								*dw++ = cos(8.0*phi);
								*dw++ = sin(8.0*phi);

								if(order >= 9)
								{
									*dw++ = cos(9.0*phi);
									*dw++ = sin(9.0*phi);

									if(order >= 10)
									{
										*dw++ = cos(10.0*phi);
										*dw++ = sin(10.0*phi);

										if(order >= 11)
										{
											*dw++ = cos(11.0*phi);
											*dw++ = sin(11.0*phi);

											if(order >= 12)
											{
												*dw++ = cos(12.0*phi);
												*dw++ = sin(12.0*phi);
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

static void ambi_decode3_encode_ls_3d(t_ambi_decode3 *x, int argc, t_atom *argv, int mode)
{
	double delta, phi;
	double cd, sd, cd2, cd3, sd2, csd, cp, sp, cp2, sp2, cp3, sp3, cp4, sp4;
	double *dw = x->x_transp;
	int index;
	int order=x->x_n_order;

	if(argc < 3)
	{
		post("ambi_decode3 ERROR: ls-input needs 1 index and 2 angles: ls index + delta [degree] + phi [degree]");
		return;
	}
	index = (int)atom_getint(argv++) - 1;
	delta = atom_getfloat(argv++);
	phi = atom_getfloat(argv);

	if(index < 0)
		index = 0;
	
	if(mode == AMBI_LS_REAL)
	{
		if(index >= x->x_n_real_ls)
			index = x->x_n_real_ls - 1;
	}
	else if(mode == AMBI_LS_PHT)
	{
		if(x->x_n_pht_ls)
		{
			if(index >= x->x_n_pht_ls)
				index = x->x_n_pht_ls - 1;
			index += x->x_n_real_ls;
		}
		else
			return;
	}
	else
		return;

	delta *= x->x_pi_over_180;
	phi *= x->x_pi_over_180;

	dw += index * x->x_n_ambi;	

	cd = cos(delta);
	sd = sin(delta);
	cp = cos(phi);
	sp = sin(phi);
	

	*dw++ = 1.0;
	*dw++ = cd * cp;
	*dw++ = cd * sp;
	*dw++ = sd;

	if(order >= 2)
	{
		cp2 = cos(2.0*phi);
		sp2 = sin(2.0*phi);
		cd2 = cd * cd;
		sd2 = sd * sd;
		csd = cd * sd;
		*dw++ = 0.5 * x->x_sqrt3 * cd2 * cp2;
		*dw++ = 0.5 * x->x_sqrt3 * cd2 * sp2;
		*dw++ = x->x_sqrt3 * csd * cp;
		*dw++ = x->x_sqrt3 * csd * sp;
		*dw++ = 0.5 * (3.0 * sd2 - 1.0);

		if(order >= 3)
		{
			cp3 = cos(3.0*phi);
			sp3 = sin(3.0*phi);
			cd3 = cd2 * cd;
			*dw++ = x->x_sqrt10_4 * cd3 * cp3;
			*dw++ = x->x_sqrt10_4 * cd3 * sp3;
			*dw++ = x->x_sqrt15_2 * cd * csd * cp2;
			*dw++ = x->x_sqrt15_2 * cd * csd * sp2;
			*dw++ = x->x_sqrt6_4 * cd * (5.0 * sd2 - 1.0) * cp;
			*dw++ = x->x_sqrt6_4 * cd * (5.0 * sd2 - 1.0) * sp;
			*dw++ = 0.5 * sd * (5.0 * sd2 - 3.0);

			if(order >= 4)
			{
				cp4 = cos(4.0*phi);
				sp4 = sin(4.0*phi);
				*dw++ = x->x_sqrt35_8 * cd2 * cd2 * cp4;
				*dw++ = x->x_sqrt35_8 * cd2 * cd2 * sp4;
				*dw++ = x->x_sqrt70_4 * cd2 * csd * cp3;
				*dw++ = x->x_sqrt70_4 * cd2 * csd * sp3;
				*dw++ = 0.5 * x->x_sqrt5_2 * cd2 * (7.0 * sd2 - 1.0) * cp2;
				*dw++ = 0.5 * x->x_sqrt5_2 * cd2 * (7.0 * sd2 - 1.0) * sp2;
				*dw++ = x->x_sqrt10_4 * csd * (7.0 * sd2 - 3.0) * cp;
				*dw++ = x->x_sqrt10_4 * csd * (7.0 * sd2 - 3.0) * sp;
				*dw++ = 0.125 * (sd2 * (35.0 * sd2 - 30.0) + 3.0);

				if(order >= 5)
				{
					*dw++ = x->x_sqrt126_16 * cd3 * cd2 * cos(5.0*phi);
					*dw++ = x->x_sqrt126_16 * cd3 * cd2 * sin(5.0*phi);
					*dw++ = x->x_sqrt315_8 * cd3 * csd * cp4;
					*dw++ = x->x_sqrt315_8 * cd3 * csd * sp4;
					*dw++ = 0.25 * x->x_sqrt70_4 * cd3 * (9.0 * sd2 - 1.0) * cp3;
					*dw++ = 0.25 * x->x_sqrt70_4 * cd3 * (9.0 * sd2 - 1.0) * sp3;
					*dw++ = x->x_sqrt105_4 * cd * csd * (3.0 * sd2 - 1.0) * cp2;
					*dw++ = x->x_sqrt105_4 * cd * csd * (3.0 * sd2 - 1.0) * sp2;
					*dw++ = 0.25 * x->x_sqrt15_2 * cd * (sd2 * (21.0 * sd2 - 14.0) + 1.0) * cp;
					*dw++ = 0.25 * x->x_sqrt15_2 * cd * (sd2 * (21.0 * sd2 - 14.0) + 1.0) * sp;
					*dw = 0.125 * sd * (sd2 * (63.0 * sd2 - 70.0) + 15.0);
				}
			}
		}
	}
}

static void ambi_decode3_real_ls(t_ambi_decode3 *x, t_symbol *s, int argc, t_atom *argv)
{
	if(x->x_n_dim == 2)
		ambi_decode3_encode_ls_2d(x, argc, argv, AMBI_LS_REAL);
	else
		ambi_decode3_encode_ls_3d(x, argc, argv, AMBI_LS_REAL);
}

static void ambi_decode3_pht_ls(t_ambi_decode3 *x, t_symbol *s, int argc, t_atom *argv)
{
	if(x->x_n_dim == 2)
		ambi_decode3_encode_ls_2d(x, argc, argv, AMBI_LS_PHT);
	else
		ambi_decode3_encode_ls_3d(x, argc, argv, AMBI_LS_PHT);
}

static void ambi_decode3_ambi_weight(t_ambi_decode3 *x, t_symbol *s, int argc, t_atom *argv)
{
	if(argc > x->x_n_order)
	{
		int i, k=0, n=x->x_n_order;
		double d;

		x->x_ambi_channel_weight[k] = atom_getfloat(argv++);
		k++;
		if(x->x_n_dim == 2)
		{
			for(i=1; i<=n; i++)
			{
				d = atom_getfloat(argv++);
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
				d = atom_getfloat(argv++);
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
		post("ambi_decode3-ERROR: ambi_weight needs %d float weights", x->x_n_order+1);
}

static void ambi_decode3_sing_range(t_ambi_decode3 *x, t_floatarg f)
{
	if(f < 0.0f)
		x->x_sing_range = -(double)f;
	else
		x->x_sing_range = (double)f;
}

static void ambi_decode3_free(t_ambi_decode3 *x)
{
	freebytes(x->x_inv_work1, x->x_n_ambi * x->x_n_ambi * sizeof(double));
	freebytes(x->x_inv_work2, 2 * x->x_n_ambi * x->x_n_ambi * sizeof(double));
	freebytes(x->x_inv_buf2, 2 * x->x_n_ambi * sizeof(double));
	freebytes(x->x_transp, (x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
	freebytes(x->x_ls_encode, (x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
	freebytes(x->x_prod, (x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
	freebytes(x->x_ambi_channel_weight, x->x_n_ambi * sizeof(double));
	freebytes(x->x_at, (x->x_n_real_ls * x->x_n_ambi + 2) * sizeof(t_atom));
}

static void *ambi_decode3_new(t_symbol *s, int argc, t_atom *argv)
{
	t_ambi_decode3 *x = (t_ambi_decode3 *)pd_new(ambi_decode3_class);
	int order, dim, i;
	int n_real_ls=0;/* number of loudspeakers */
	int n_pht_ls=0;/* number of phantom_loudspeakers */

	if((argc >= 4) &&
		IS_A_FLOAT(argv,0) &&
		IS_A_FLOAT(argv,1) &&
		IS_A_FLOAT(argv,2) &&
		IS_A_FLOAT(argv,3))
	{
		order = (int)atom_getint(argv++);
		dim = (int)atom_getint(argv++);
		n_real_ls = (int)atom_getint(argv++);
		n_pht_ls = (int)atom_getint(argv);

		if(order < 1)
			order = 1;
		if(dim != 3)
		{
			dim = 2;
			if(order > 12)
				order = 12;
			x->x_n_ambi = 2*order + 1;
		}
		else
		{
			if(order > 5)
				order = 5;
			x->x_n_ambi = (order + 1)*(order + 1);
		}
		x->x_n_dim = dim;
		x->x_n_order = order;
		if(n_real_ls < 1)
			n_real_ls = 1;
		if(n_pht_ls < 0)
			n_pht_ls = 0;
		if((n_real_ls + n_pht_ls) < x->x_n_ambi)
			post("ambi_decode3-WARNING: Number of Loudspeakers < Number of Ambisonic-Channels !!!!");
		
		x->x_n_real_ls = n_real_ls;
		x->x_n_pht_ls = n_pht_ls;
		x->x_inv_work1 = (double *)getbytes(x->x_n_ambi * x->x_n_ambi * sizeof(double));
		x->x_inv_work2 = (double *)getbytes(2 * x->x_n_ambi * x->x_n_ambi * sizeof(double));
		x->x_inv_buf2 = (double *)getbytes(2 * x->x_n_ambi * sizeof(double));
		x->x_transp = (double *)getbytes((x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
		x->x_ls_encode = (double *)getbytes((x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
		x->x_prod = (double *)getbytes((x->x_n_real_ls+x->x_n_pht_ls) * x->x_n_ambi * sizeof(double));
		x->x_ambi_channel_weight = (double *)getbytes(x->x_n_ambi * sizeof(double));
		x->x_at = (t_atom *)getbytes((x->x_n_real_ls * x->x_n_ambi + 2) * sizeof(t_atom));
		x->x_s_matrix = gensym("matrix");
		/*change*/
		SETFLOAT(x->x_at, (t_float)x->x_n_real_ls);
		SETFLOAT(x->x_at+1, (t_float)x->x_n_ambi);

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
		for(i=0; i<x->x_n_ambi; i++)
			x->x_ambi_channel_weight[i] = 1.0;
		outlet_new(&x->x_obj, &s_list);
		return (x);
	}
	else
	{
		post("ambi_decode3-ERROR: need 4 float arguments: ambi_order dimension number_of_real_loudspeakers number_of_canceled_phantom_speakers");
		return(0);
	}
}

void ambi_decode3_setup(void)
{
	ambi_decode3_class = class_new(gensym("ambi_decode3"), (t_newmethod)ambi_decode3_new, (t_method)ambi_decode3_free,
				   sizeof(t_ambi_decode3), 0, A_GIMME, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_real_ls, gensym("real_ls"), A_GIMME, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_pht_ls, gensym("pht_ls"), A_GIMME, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_ambi_weight, gensym("ambi_weight"), A_GIMME, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_sing_range, gensym("sing_range"), A_DEFFLOAT, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_begin_pseudo_inverse, gensym("begin_pseudo_inverse"), 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_ipht_ireal_muladd, gensym("ipht_ireal_muladd"), A_GIMME, 0);
	class_addmethod(ambi_decode3_class, (t_method)ambi_decode3_end_pseudo_inverse, gensym("end_pseudo_inverse"), 0);
//	class_sethelpsymbol(ambi_decode3_class, gensym("iemhelp2/ambi_decode3-help"));
}
