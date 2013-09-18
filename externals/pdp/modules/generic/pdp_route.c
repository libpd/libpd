/*
 *   Pure Data Packet module.
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



#include "pdp.h"
#include "pdp_internals.h"

// dynamic ????
#define PDP_ROUTE_MAX_NB_OUTLETS 100

typedef struct pdp_route_struct
{
    t_object x_obj;
    t_float x_f;

    t_outlet *x_outlet[PDP_ROUTE_MAX_NB_OUTLETS];

    int x_nb_outlets;
    int x_route;
    int x_route_next;


} t_pdp_route;


static void pdp_route_input_0(t_pdp_route *x, t_symbol *s, t_floatarg f)
{
    t_atom atom[2];
    t_symbol *pdp = gensym("pdp");


    /* trigger on register_ro */
    if (s == gensym("register_ro")){
	x->x_route = x->x_route_next;
    }

    /* propagate the pdp message */
    SETSYMBOL(atom+0, s);
    SETFLOAT(atom+1, f);
    outlet_anything(x->x_outlet[x->x_route], pdp, 2, atom);

}

static void pdp_route_input_0_dpd(t_pdp_route *x, t_symbol *s, t_floatarg f)
{

    /* trigger on accumulate */
    if (s == gensym("accumulate")){
	x->x_route = x->x_route_next;
    }

    /* propagate the dpd message */
    outlet_dpd(x->x_outlet[x->x_route], (int)f);

}



static void pdp_route_route(t_pdp_route *x, t_floatarg f)
{
    int route = (int)f;

    if (route < 0) route = 0;
    if (route >= x->x_nb_outlets) route = x->x_nb_outlets - 1;
    
    x->x_route_next = route;

}



static void pdp_route_free(t_pdp_route *x)
{
    
}

t_class *pdp_route_class;



void *pdp_route_new(t_floatarg f)
{
    int nboutlets = (int)f;
    int i;

    t_pdp_route *x = (t_pdp_route *)pd_new(pdp_route_class);


    if (nboutlets < 2) nboutlets = 2;
    if (nboutlets >= PDP_ROUTE_MAX_NB_OUTLETS) nboutlets = PDP_ROUTE_MAX_NB_OUTLETS - 1;


    inlet_new(&x->x_obj, &x->x_obj.ob_pd, gensym("float"), gensym("route"));

    x->x_nb_outlets = nboutlets;
    x->x_route = 0;
    x->x_route_next = 0;

    for (i=0; i<nboutlets; i++) 
	x->x_outlet[i] = outlet_new(&x->x_obj, &s_anything);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_route_setup(void)
{


    pdp_route_class = class_new(gensym("pdp_route"), (t_newmethod)pdp_route_new,
    	(t_method)pdp_route_free, sizeof(t_pdp_route), 0, A_DEFFLOAT, A_NULL);


    class_addmethod(pdp_route_class, (t_method)pdp_route_input_0, gensym("pdp"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_route_class, (t_method)pdp_route_input_0_dpd, gensym("dpd"),  A_SYMBOL, A_DEFFLOAT, A_NULL);
    class_addmethod(pdp_route_class, (t_method)pdp_route_route, gensym("route"),  A_DEFFLOAT, A_NULL);

}

#ifdef __cplusplus
}
#endif
