/*
 *   Pure Data Packet module. Vector modules.
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


typedef struct pdp_mat_vec_struct
{
    t_pdp_base x_base;
    int x_type;
    t_outlet *x_out;
    int x_accept_list;
    int x_list_size;
    t_atom *x_list;

} t_pdp_mat_vec;


#define GETFLOAT(x) ((x)->a_type == A_FLOAT ? (x)->a_w.w_float : 0.0f)
#define GETDOUBLE(x) (double)GETFLOAT(x)


static void pdp_mat_vec_list_in(t_pdp_mat_vec *x, t_symbol *s, int argc, t_atom *argv)
{
    int i;
    int vp = -1;
    int f;
    int dim = argc;

    if (!x->x_accept_list) return; //check if this handler is enabled
    if (!argc) return; //reject empty list

    switch(x->x_type){
    case PDP_MATRIX_TYPE_CFLOAT:
	if (argc & 1) return; //reject odd nb elements
	dim >>= 1; //halve dimension
    case PDP_MATRIX_TYPE_RFLOAT:
	vp = pdp_packet_new_matrix(dim, 1, x->x_type);
	if (-1 != vp){
	    float *data = (float *)pdp_packet_data(vp);
	    for (i=0; i<argc; i++) data[i] = GETFLOAT(&argv[i]);
	}
	break;
    case PDP_MATRIX_TYPE_CDOUBLE:
	if (argc & 1) return; //reject odd nb elements
	dim >>= 1; //halve dimension
    case PDP_MATRIX_TYPE_RDOUBLE:
	vp = pdp_packet_new_matrix(dim, 1, x->x_type);
	if (-1 != vp){
	    double *data = (double *)pdp_packet_data(vp);
	    for (i=0; i<argc; i++) data[i] = GETDOUBLE(&argv[i]);
	}
	break;
    default:
	break;
    }

    if (-1 != vp){
	/* store vector packet */
	pdp_base_set_packet(x, 0, vp);
	pdp_base_bang(x);
    }
}

static void pdp_mat_vec_list_out(t_pdp_mat_vec *x)
{
    int p = pdp_base_move_packet(x, 0);
    int type = pdp_packet_matrix_get_type(p);
    int outlist_size;
    float *fdata = 0;
    double *ddata = 0;
    int i;

    /* check if it's a vector */
    gsl_vector *m = (gsl_vector *)pdp_packet_matrix_get_gsl_vector(p, type);
    if (!pdp_packet_matrix_isvector(p)) return;

    /* get list size */
    outlist_size = m->size;
    if ((type == PDP_MATRIX_TYPE_CFLOAT)
	|| (type == PDP_MATRIX_TYPE_CDOUBLE)) 
	outlist_size <<= 1;
    
    /* realloc list if necessary */
    if (outlist_size > x->x_list_size){
	free(x->x_list);
	x->x_list = (t_atom *)pdp_alloc(sizeof(t_atom) * outlist_size);
	x->x_list_size = outlist_size;
    }

    /* copy data */
    switch(type){
    case PDP_MATRIX_TYPE_RFLOAT:
    case PDP_MATRIX_TYPE_CFLOAT:
	fdata = (float *)pdp_packet_data(p);
	for (i=0; i<outlist_size; i++)
	    SETFLOAT(&x->x_list[i], fdata[i]);
	break;
	
    case PDP_MATRIX_TYPE_RDOUBLE:
    case PDP_MATRIX_TYPE_CDOUBLE:
	ddata = (double *)pdp_packet_data(p);
	for (i=0; i<outlist_size; i++)
	    SETFLOAT(&x->x_list[i], (float)ddata[i]);
	break;
	
    }	
			    
    /* dispose of vector packet and output list */
    pdp_packet_mark_unused(p);
    outlet_list(x->x_out, &s_list, outlist_size, x->x_list);
}

static void pdp_mat_vec_free(t_pdp_mat_vec *x)
{
    /* remove process method from queue before deleting data */
    pdp_base_free(x);

    /* delete list */
    if (x->x_list) pdp_dealloc (x->x_list);
}

t_class *pdp_mat_vec_class;


/* common new methods */
t_pdp_mat_vec *pdp_mat_vec_base_new(void)
{
    t_pdp_mat_vec *x = (t_pdp_mat_vec *)pd_new(pdp_mat_vec_class);
    pdp_base_init(x);
    x->x_type = PDP_MATRIX_TYPE_CFLOAT;
    x->x_accept_list = 0;
    x->x_list_size = 0;
    x->x_list = 0;
    return x;
}

void *pdp_mat_vec_list2vec_new(t_symbol *type)
{
    t_pdp_mat_vec *x = pdp_mat_vec_base_new();
    pdp_base_disable_active_inlet(x);
    pdp_base_add_pdp_outlet(x);
    x->x_accept_list = 1;
    if ((gensym ("") == type) || (gensym ("double/real") == type)) x->x_type = PDP_MATRIX_TYPE_RDOUBLE;
    else if (gensym ("double/complex") == type) x->x_type = PDP_MATRIX_TYPE_CDOUBLE;
    else if (gensym ("float/real") == type) x->x_type = PDP_MATRIX_TYPE_RFLOAT;
    else if (gensym ("float/complex") == type) x->x_type = PDP_MATRIX_TYPE_CFLOAT;
    else {
	pd_free((t_pd *)x);
	x = 0;
    }
    return (void *)x;
}


void *pdp_mat_vec_vec2list_new(t_symbol *type)
{
    t_pdp_mat_vec *x = pdp_mat_vec_base_new();
    x->x_out = outlet_new((t_object *)x, &s_anything);
    pdp_base_set_postproc_method(x,(t_pdp_method)pdp_mat_vec_list_out);
    pdp_base_readonly_active_inlet(x);
    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_mat_vec_setup(void)
{


    pdp_mat_vec_class = class_new(gensym("pdp_m_list2vec"), (t_newmethod)pdp_mat_vec_list2vec_new,
    	(t_method)pdp_mat_vec_free, sizeof(t_pdp_mat_vec), 0, A_DEFSYMBOL, A_NULL);

    pdp_base_setup(pdp_mat_vec_class);


    class_addcreator((t_newmethod)pdp_mat_vec_vec2list_new, gensym("pdp_m_vec2list"), A_NULL);

    class_addlist(pdp_mat_vec_class, (t_method)pdp_mat_vec_list_in);
 

}

#ifdef __cplusplus
}
#endif
