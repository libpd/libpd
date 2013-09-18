/*
 *   Pure Data Packet module. LU decomposition module.
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

//#include <gsl/gsl_block.h>
//#include <gsl/gsl_vector.h>
//#include <gsl/gsl_matrix.h>
//#include <gsl/gsl_blas.h>
#include "pdp.h"
#include "pdp_base.h"


typedef struct pdp_mat_LU_struct
{
    t_pdp_base x_base;

} t_pdp_mat_LU;



static void pdp_mat_LU_process_LU_inverse(t_pdp_mat_LU *x)
{
    int p = pdp_base_get_packet(x, 0);
    int p_LU = pdp_packet_matrix_LU_to_inverse(p);
    pdp_base_set_packet(x, 0, p_LU); // replace packet
}

static void pdp_mat_LU_process_LU(t_pdp_mat_LU *x)
{
    int p = pdp_base_get_packet(x, 0);
    int p_LU = pdp_packet_matrix_LU(p);
    pdp_base_set_packet(x, 0, p_LU); // replace packet
}

static void pdp_mat_LU_process_LU_solve(t_pdp_mat_LU *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    int pvr, pm, pv;

    /* determine which is vector and which is matrix */
    if (pdp_packet_matrix_ismatrix(p0) && pdp_packet_matrix_isvector(p1)){
	pm = p0;
	pv = p1;
    }
    else {
	pm = p1;
	pv = p0;
    }

    /* create the result vector */
    pvr = pdp_packet_matrix_LU_solve(pm, pv);

    /* replace the active packet */
    pdp_base_set_packet(x, 0, pvr);
}

static void pdp_mat_LU_free(t_pdp_mat_LU *x)
{
    /* remove process method from queue before deleting data */
    pdp_base_free(x);
}

t_class *pdp_mat_LU_class;


/* common new methods */
t_pdp_mat_LU *pdp_mat_LU_base_new(void)
{
    t_pdp_mat_LU *x = (t_pdp_mat_LU *)pd_new(pdp_mat_LU_class);
    pdp_base_init(x);
    pdp_base_add_pdp_outlet(x);
    return x;
}

void *pdp_mat_LU_inverse_new(void)
{
    t_pdp_mat_LU *x = pdp_mat_LU_base_new();
    pdp_base_set_process_method(x,(t_pdp_method)pdp_mat_LU_process_LU_inverse);
    pdp_base_readonly_active_inlet(x);
    return (void *)x;
}


void *pdp_mat_LU_new(void)
{
    t_pdp_mat_LU *x = pdp_mat_LU_base_new();
    pdp_base_set_process_method(x,(t_pdp_method)pdp_mat_LU_process_LU);
    pdp_base_readonly_active_inlet(x);
    return (void *)x;
}

void *pdp_mat_LU_solve_new(void)
{
    t_pdp_mat_LU *x = pdp_mat_LU_base_new();
    pdp_base_set_process_method(x,(t_pdp_method)pdp_mat_LU_process_LU_solve);
    pdp_base_readonly_active_inlet(x);
    pdp_base_add_pdp_inlet(x);
    return (void *)x;
}

#ifdef __cplusplus
extern "C"
{
#endif


void pdp_mat_lu_setup(void)
{


    pdp_mat_LU_class = class_new(gensym("pdp_m_LU_inverse"), (t_newmethod)pdp_mat_LU_inverse_new,
    	(t_method)pdp_mat_LU_free, sizeof(t_pdp_mat_LU), 0, A_NULL);

    pdp_base_setup(pdp_mat_LU_class);


    class_addcreator((t_newmethod)pdp_mat_LU_new, gensym("pdp_m_LU"), A_NULL);
    class_addcreator((t_newmethod)pdp_mat_LU_solve_new, gensym("pdp_m_LU_solve"), A_NULL);

 

}

#ifdef __cplusplus
}
#endif
