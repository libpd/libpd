/*
 *   Pure Data Packet system implementation. matrix packet interface
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

#ifndef PDP_MATRIX_H
#define PDP_MATRIX_H

#include <stdio.h>
#include <gsl/gsl_block.h>
#include <gsl/gsl_vector.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h> 

#include "pdp_types.h"

#define gsl_rows size1
#define gsl_columns size2

typedef struct _matrix
{
    /* meta data */
    u32 type;                 /* float/double real/complex */
    u32 rows;
    u32 columns;

    /* gsl structures: these will be cast to the correct type on use */
    gsl_block  block;     /* gsl block meta data */
    gsl_vector vector;    /* gsl vector meta data */
    gsl_matrix matrix;    /* gsl matrix meta data */
    gsl_permutation perm; /* permutation data for storing an LU decomposition */
    int signum;           /* sign of permutation matrix */


} t_matrix;

#define PDP_MATRIX 7

#define PDP_MATRIX_TYPE_RFLOAT 1
#define PDP_MATRIX_TYPE_CFLOAT 2
#define PDP_MATRIX_TYPE_RDOUBLE 3
#define PDP_MATRIX_TYPE_CDOUBLE 4

int pdp_packet_matrix_isvalid(int p);
int pdp_packet_matrix_isvector(int p);
int pdp_packet_matrix_ismatrix(int p);

int pdp_packet_new_matrix(u32 rows, u32 columns, u32 type);
int pdp_packet_new_matrix_product_result(CBLAS_TRANSPOSE_t TransA, CBLAS_TRANSPOSE_t TransB, int pA, int pB);
void pdp_packet_matrix_setzero(int p);


/* getters: returns 0 if type is incorrect */
void *pdp_packet_matrix_get_gsl_matrix(int p, u32 type);
void *pdp_packet_matrix_get_gsl_vector(int p, u32 type);
int pdp_packet_matrix_get_type(int p);


/* type transparent matrix operations */

/* blas wrappers */

/* C += scale op(A) op(B) */
int pdp_packet_matrix_blas_mm(CBLAS_TRANSPOSE_t TransA, CBLAS_TRANSPOSE_t TransB, 
			      int pA, int pB, int pC,
			      float scale_r, float scale_i);
/* c += scale op(A) b */
int pdp_packet_matrix_blas_mv(CBLAS_TRANSPOSE_t TransA,
			      int pA, int pb, int pc, 
			      float scale_r, float scale_i);

/* other gsl wrappers */
int pdp_packet_matrix_LU(int p_matrix);
int pdp_packet_matrix_LU_to_inverse(int p_matrix);
int pdp_packet_matrix_LU_solve(int p_matrix, int p_vector);


#endif
