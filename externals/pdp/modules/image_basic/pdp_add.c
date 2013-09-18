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

typedef struct pdp_add_struct
{
    /* a pdp derived class has the t_pdp_imagebase data member as first entry */
    /* it contains the pd object and the data members for the pdp_base class */
    t_pdp_imagebase x_base;

} t_pdp_add;


/* the process method */
static void pdp_add_process(t_pdp_add *x)
{
    /* get received packets */
    int p0, p1;

    /* get channel mask */
    int mask = pdp_imagebase_get_chanmask(x);

    /* this processes the packets using a pdp image processor */
    /* replace this with your own processing code */
    /* for raw packet acces: use pdp_pacjet_header() and pdp_packet_data() */
    p0 = pdp_base_get_packet(x,0);
    p1 = pdp_base_get_packet(x,1);

    pdp_imageproc_dispatch_2buf(&pdp_imageproc_add_process, 0, mask, p0, p1);


}


static void pdp_add_free(t_pdp_add *x)
{
    /* free super: this is mandatory 
       (it stops the thread if there is one running and frees all packets) */
    pdp_imagebase_free(x);

    /* if you have allocated more packets
       this is the place to free them with pdp_mark_unused */
}

t_class *pdp_add_class;


void *pdp_add_new(void)
{
    /* allocate */
    t_pdp_add *x = (t_pdp_add *)pd_new(pdp_add_class);

    /* init super: this is mandatory */
    pdp_imagebase_init(x);

    /* set the pdp processing method */
    pdp_base_set_process_method(x, (t_pdp_method)pdp_add_process);

    /* create additional cold (readonly) pdp inlets (there is already one pdp inlet) */
    pdp_base_add_pdp_inlet(x);

    /* create a pdp_outlet */
    pdp_base_add_pdp_outlet(x);

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_add_setup(void)
{
    /* create a standard pd class */
    pdp_add_class = class_new(gensym("pdp_add"), (t_newmethod)pdp_add_new,
   	(t_method)pdp_add_free, sizeof(t_pdp_add), 0, A_NULL);

    /* inherit pdp base class methods */
    pdp_imagebase_setup(pdp_add_class);
}

#ifdef __cplusplus
}
#endif
