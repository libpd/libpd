
/*
 *   Pure Data Packet. Header file for mmx routines.
 *   Copyright (c) by Tom Schouten <tom@zwizwa.be>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */


#ifndef PDP_MMX_H
#define PDP_MMX_H

#ifdef __cplusplus
extern "C"
{
#endif

/****************************** 16 bit signed (pixel) routines ***************************************/

/* pack: gain is 8.8 fixed point */
void pixel_pack_s16u8_y(short int *input_pixels, 
			unsigned char *output_pixels, 
			int nb_pixels_div_8);

void pixel_pack_s16u8_uv(short int *input_pixels, 
			 unsigned char *output_pixels, 
			 int nb_pixels_div_8);


/* unpack: gain is not used -> full scale unpack */
void pixel_unpack_u8s16_y(unsigned char *input_pixels, 
			  short int *output_pixels, 
			  int nb_pixels_div_8);

void pixel_unpack_u8s16_uv(unsigned char *input_pixels, 
			   short int *output_pixels, 
			   int nb_pixels_div_8);


/* gain */
/* gain = integer */
/* shift is down shift count */
void pixel_gain_s16(short int *image, 
		    int nb_4pixel_vectors, 
		    short int gain[4], 
		    unsigned long long *shift);


/* mix: left = gain_left * left + gain_right * right / gains are s.15 fixed point */
void pixel_mix_s16(short int *left, 
		   short int *right, 
		   int nb_4pixel_vectors,
		   short int gain_left[4], 
		   short int gain_right[4]);

void pixel_randmix_s16(short int *left, 
		       short int *right, 
		       int nb_4pixel_vectors, 
		       short int random_seed[4], 
		       short int threshold[4]);

void pixel_rand_s16(short int *image, 
		    int nb_4pixel_vectors, 
		    short int random_seed[4]);

void pixel_add_s16(short int *left, 
		   short int *right, 
		   int nb_4pixel_vectors);

void pixel_mul_s16(short int *left, 
		   short int *right, 
		   int nb_4pixel_vectors);


/* affine transfo */
void pixel_affine_s16(short int *buf, 
		      int nb_4pixel_vectors, 
		      short int gain[4], 
		      short int offset[4]);

/* conv */
void pixel_conv_hor_s16(short int *pixel_array, 
			int nb_4_pixel_vectors, 
			short int border[4],
			short int mask[12]);

void pixel_conv_ver_s16(short int *pixel_array, 
			int nb_4_pixel_vectors, 
			int row_byte_size, 
			short int border[4],
			short int mask[12]);

/* biquad */

void pixel_biquad_vertb_s16(short int *pixel_array, 
			    int nb_4x4_pixblocks, 
			    int linewidth, 
			    short int coef[20], 
			    short int state[8]);

void pixel_biquad_verbt_s16(short int *pixel_array, 
			    int nb_4x4_pixblocks, 
			    int linewidth, 
			    short int coef[20], 
			    short int state[8]);


void pixel_biquad_horlr_s16(short int *pixel_array, 
			    int nb_4x4_pixblocks, 
			    int linewidth, 
			    short int coef[20], 
			    short int state[8]);

void pixel_biquad_horrl_s16(short int *pixel_array, 
			    int nb_4x4_pixblocks, 
			    int linewidth, 
			    short int coef[20], 
			    short int state[8]);

void pixel_biquad_time_s16(short int *pixel_array, 
			   short int *state_array1, 
			   short int *state_array2, 
			   short int *coefs, 
			   int nb_4_pix_vectors);

/********************************** PLANAR COLOUR OPERATIONS ***************************************/

/* color rotation for 3 colour planes */
void pixel_crot3d_s16(short int *pixel_array, 
		      int nb_4pixel_vectors_per_plane, 
		      short int *row_encoded_vector_matrix);


/* color rotation for 2 colour planes */
void pixel_crot2d_s16(short int *pixel_array, 
		      int nb_4pixel_vectors_per_plane, 
		      short int *row_encoded_vector_matrix);


/********************************** RESAMPLE OPERATIONS *******************************************/

// affine transformation (called linear map, but that's flou terminology)
void pixel_resample_linmap_s16(void *x);



/********************************** POLYNOMIAL OPERATIONS *****************************************/
// chebychev polynomial
void pixel_cheby_s16_3plus(short int *buf, int nb_8pixel_vectors, int orderplusone, short int *coefs);



#ifdef __cplusplus
}
#endif


#endif //PDP_MMX_H
