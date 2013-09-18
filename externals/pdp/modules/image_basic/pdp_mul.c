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
#include "pdp_imagebase.h"

typedef struct pdp_mul_struct
{
    t_pdp_imagebase x_base;

} t_pdp_mul;



static void pdp_mul_process(t_pdp_mul *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_mul_process, 0, mask, p0, p1);
}



static void pdp_mul_free(t_pdp_mul *x)
{
    pdp_imagebase_free(x);
}

t_class *pdp_mul_class;



void *pdp_mul_new(void)
{
    t_pdp_mul *x = (t_pdp_mul *)pd_new(pdp_mul_class);
    
    /* super init */
    pdp_imagebase_init(x);
    pdp_base_add_pdp_inlet(x); 
    pdp_base_add_pdp_outlet(x);
    pdp_base_set_process_method(x, (t_pdp_method)pdp_mul_process);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_mul_setup(void)
{


    pdp_mul_class = class_new(gensym("pdp_mul"), (t_newmethod)pdp_mul_new,
    	(t_method)pdp_mul_free, sizeof(t_pdp_mul), 0, A_NULL);

    pdp_imagebase_setup(pdp_mul_class);
}

#ifdef __cplusplus
}
#endif
