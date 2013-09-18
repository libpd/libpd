/*
 *   Pure Data Packet module. Matrix multiplication module
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


typedef struct pdp_mat_mm_struct
{
    t_pdp_base x_base;
    CBLAS_TRANSPOSE_t x_T0;
    CBLAS_TRANSPOSE_t x_T1;
    int x_M0;
    int x_M1;

    float x_scale_r;
    float x_scale_i;

} t_pdp_mat_mm;


static void pdp_mat_mm_rscale(t_pdp_mat_mm *x, t_floatarg r)
{
    x->x_scale_r = r;
    x->x_scale_i = 0.0f;
}

static void pdp_mat_mm_cscale(t_pdp_mat_mm *x, t_floatarg r, t_floatarg i)
{
    x->x_scale_r = r;
    x->x_scale_i = i;
}


/* matrix multilpy */
static void pdp_mat_mv_process_mul(t_pdp_mat_mm *x)
{
    int pA = pdp_base_get_packet(x, 0);
    int pB = pdp_base_get_packet(x, 1);
    int p0, p1, pR;

    /* determine which one is the vector */
    if (pdp_packet_matrix_isvector(pA)){
	p0 = pB;
	p1 = pA;
    }
    else {
	p1 = pB;
	p0 = pA;
    }

    pR = pdp_packet_new_matrix_product_result(x->x_T0, CblasNoTrans, p0, p1);

    if (-1 != pR){
	pdp_packet_matrix_setzero(pR);
	if (pdp_packet_matrix_blas_mv(x->x_T0, p0, p1, pR, x->x_scale_r, x->x_scale_i)){
	    //post("pdp_packet_matrix_blas_mm failed");
	    pdp_packet_mark_unused(pR);
	    pR = -1;
	}
    }
    else {
	//post("pdp_packet_new_matrix_product_result failed");
    }

    /* replace with result */
    pdp_base_set_packet(x, 0, pR);

}

/* matrix vector multilpy */
static void pdp_mat_mm_process_mul(t_pdp_mat_mm *x)
{
    int pA = pdp_base_get_packet(x, 0);
    int pB = pdp_base_get_packet(x, 1);
    int p0, p1, pR;

    p0 = (x->x_M0) ? pB : pA;
    p1 = (x->x_M1) ? pB : pA;

    pR = pdp_packet_new_matrix_product_result(x->x_T0, x->x_T1, p0, p1);

    if (-1 != pR){
	pdp_packet_matrix_setzero(pR);
	if (pdp_packet_matrix_blas_mm(x->x_T0, x->x_T1, p0, p1, pR, x->x_scale_r, x->x_scale_i)){
	    //post("pdp_packet_matrix_blas_mm failed");
	    pdp_packet_mark_unused(pR);
	    pR = -1;
	}
    }
    else {
	//post("pdp_packet_new_matrix_product_result failed");
    }

    /* replace with result */
    pdp_base_set_packet(x, 0, pR);

}
/* matrix macc */
static void pdp_mat_mm_process_mac(t_pdp_mat_mm *x)
{
    int pC = pdp_base_get_packet(x, 0);
    int pA = pdp_base_get_packet(x, 1);
    int pB = pdp_base_get_packet(x, 2);
    int p0, p1;

    p0 = (x->x_M0) ? pB : pA;
    p1 = (x->x_M1) ? pB : pA;

    if (pdp_packet_matrix_blas_mm(x->x_T0, x->x_T1, p0, p1, pC, x->x_scale_r, x->x_scale_i)){
	//post("pdp_packet_matrix_blas_mm failed");
	pdp_base_set_packet(x, 0, -1); // delete packet
    }

}


static void pdp_mat_mm_free(t_pdp_mat_mm *x)
{
    /* remove process method from queue before deleting data */
    pdp_base_free(x);
}

t_class *pdp_mat_mm_class;


/* common new method */
void *pdp_mat_mm_new(void)
{
    int i;
    t_pdp_mat_mm *x = (t_pdp_mat_mm *)pd_new(pdp_mat_mm_class);
 
    /* super init */
    pdp_base_init(x);

    /* outlet */
    pdp_base_add_pdp_outlet(x);


    return (void *)x;
}


static int pdp_mat_mm_setup_routing_M0(t_pdp_mat_mm *x, t_symbol *s0)
{
     if ('A' == s0->s_name[0]){x->x_M0 = 0;} else if ('B' == s0->s_name[0]) {x->x_M0 = 1;} else return 0;

     if ((gensym("A") == s0) || (gensym("B") == s0)) x->x_T0 = CblasNoTrans;
     else if ((gensym("A^T") == s0) || (gensym("B^T") == s0)) x->x_T0 = CblasConjTrans;
     else if ((gensym("A^H") == s0) || (gensym("B^H") == s0)) x->x_T0 = CblasConjTrans;
     else return 0;

     return 1;
}

static int pdp_mat_mm_setup_routing_M1(t_pdp_mat_mm *x, t_symbol *s1)
{

     if ('A' == s1->s_name[0]){x->x_M1 = 0;} else if ('B' == s1->s_name[0]) {x->x_M1 = 1;} else return 0;

     /* setup second matrix transpose operation */
     if ((gensym("A") == s1) || (gensym("B") == s1)) x->x_T1 = CblasNoTrans;
     else if ((gensym("A^T") == s1) || (gensym("B^T") == s1)) x->x_T1 = CblasConjTrans;
     else if ((gensym("A^H") == s1) || (gensym("B^H") == s1)) x->x_T1 = CblasConjTrans;
     else return 0;

     return 1;
}


static int pdp_mat_mm_setup_scaling(t_pdp_mat_mm *x, t_symbol *scale)
{
    int success = 1;

     /* setup scaling inlet */
     if ((gensym ("rscale") == scale) || (gensym("r") == scale)){
	 pdp_base_add_gen_inlet(x, gensym("float"), gensym("rscale"));
     }
     else if ((gensym ("cscale") == scale) || (gensym("c") == scale)){
	 pdp_base_add_gen_inlet(x, gensym("list"), gensym("cscale"));
     }
     else if (gensym ("") != scale) success = 0;
     
     return success;
}

void *pdp_mat_mm_new_mul_common(t_symbol *s0, t_symbol *s1, t_symbol *scale, int ein)
{
     t_pdp_mat_mm *x = pdp_mat_mm_new();

     /* add extra pdp inlets */
     while (ein--) pdp_base_add_pdp_inlet(x);

     /* setup routing */
     if (!pdp_mat_mm_setup_routing_M0(x, s0)) goto error;
     if (!pdp_mat_mm_setup_routing_M1(x, s1)) goto error;
     if (!pdp_mat_mm_setup_scaling(x, scale)) goto error;

     /* default scale = 1 */
     pdp_mat_mm_cscale(x, 1.0f, 0.0f);
     return (void *)x;

 error:
     pd_free((void *)x);
     return 0;
}

void *pdp_mat_mv_new_mul_common(t_symbol *s0, t_symbol *scale, int ein)
{
     t_pdp_mat_mm *x = pdp_mat_mm_new();

     /* add extra pdp inlets */
     while (ein--) pdp_base_add_pdp_inlet(x);

     /* setup routing */
     if (!pdp_mat_mm_setup_routing_M0(x, s0)) goto error;
     if (!pdp_mat_mm_setup_scaling(x, scale)) goto error;

     /* default scale = 1 */
     pdp_mat_mm_cscale(x, 1.0f, 0.0f);
     return (void *)x;

 error:
     pd_free((void *)x);
     return 0;
}

void *pdp_mat_mm_new_mul(t_symbol *s0, t_symbol *s1, t_symbol *scale)
{
    t_pdp_mat_mm *x = pdp_mat_mm_new_mul_common(s0, s1, scale, 1);
    if(x){
	pdp_base_set_process_method(x, (t_pdp_method)pdp_mat_mm_process_mul);
	pdp_base_readonly_active_inlet(x);
    }
    return x;
}

void *pdp_mat_mv_new_mul(t_symbol *s0, t_symbol *scale)
{
    t_pdp_mat_mm *x = pdp_mat_mv_new_mul_common(s0, scale, 1);
    if(x){
	pdp_base_set_process_method(x, (t_pdp_method)pdp_mat_mv_process_mul);
	pdp_base_readonly_active_inlet(x);
    }
    return x;
}

void *pdp_mat_mm_new_mac(t_symbol *s0, t_symbol *s1, t_symbol *scale)
{
    t_pdp_mat_mm *x = pdp_mat_mm_new_mul_common(s0, s1, scale, 2);
    if (x){
	pdp_base_set_process_method(x, (t_pdp_method)pdp_mat_mm_process_mac);
    }
    return x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_mat_mul_setup(void)
{


    pdp_mat_mm_class = class_new(gensym("pdp_m_mm"), (t_newmethod)pdp_mat_mm_new_mul,
    	(t_method)pdp_mat_mm_free, sizeof(t_pdp_mat_mm), 0, A_SYMBOL, A_SYMBOL, A_DEFSYMBOL, A_NULL);

    pdp_base_setup(pdp_mat_mm_class);

    class_addcreator((t_newmethod)pdp_mat_mm_new_mac, gensym("pdp_m_+=mm"),
		     A_SYMBOL, A_SYMBOL, A_DEFSYMBOL, A_NULL);
  
    class_addcreator((t_newmethod)pdp_mat_mv_new_mul, gensym("pdp_m_mv"),
		     A_SYMBOL, A_DEFSYMBOL, A_NULL);
  

    class_addmethod(pdp_mat_mm_class, (t_method)pdp_mat_mm_rscale, gensym("rscale"), A_FLOAT, A_NULL);
    class_addmethod(pdp_mat_mm_class, (t_method)pdp_mat_mm_cscale, gensym("cscale"), A_FLOAT, A_FLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
