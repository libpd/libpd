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

typedef struct pdp_logic_struct
{
    t_pdp_imagebase x_base;

    void *x_mask;
   
} t_pdp_logic;


static void pdp_logic_process_and(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_and_process, 0, mask, p0, p1);
}

static void pdp_logic_process_or(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_or_process, 0, mask, p0, p1);
}

static void pdp_logic_process_xor(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    int p1 = pdp_base_get_packet(x, 1);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_2buf(&pdp_imageproc_xor_process, 0, mask, p0, p1);
}

static void pdp_logic_process_not(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_not_process, 0, mask, p0);
}

static void pdp_logic_process_mask(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_mask_process, x->x_mask, mask, p0);
}

static void pdp_logic_process_softthresh(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_softthresh_process, x->x_mask, mask, p0);
}


static void pdp_logic_process_hardthresh(t_pdp_logic *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_hardthresh_process, x->x_mask, mask, p0);
}


static void pdp_logic_set_mask(t_pdp_logic *x, t_floatarg f)
{
    /* using a pointer as a variable hmm? */
    sptr mask = ((sptr)f) & 0xffff;
    x->x_mask = ((void * )mask);
}

static void pdp_logic_set_threshold(t_pdp_logic *x, t_floatarg f)
{
    /* using a pointer as a variable hmm? */
    if (f<0.0f) f = 0.0f;
    if (f>1.0f) f = 1.0f;
    x->x_mask = (void *)((sptr)(((float)0x7fff) * f));
}

static void pdp_logic_set_depth(t_pdp_logic *x, t_floatarg f)
{
    sptr mask;
    int shift = (16 - ((int)f));
    if (shift < 0) shift = 0;
    if (shift > 16) shift = 16;
    mask =  ((0xffff)<<shift) & 0xffff;
    x->x_mask = (void *)mask;

}


static void pdp_logic_free(t_pdp_logic *x)
{
    /* remove process method from queue before deleting data */
    pdp_imagebase_free(x);
}

t_class *pdp_logic_class;


/* common new method */
void *pdp_logic_new(void)
{
    t_pdp_logic *x = (t_pdp_logic *)pd_new(pdp_logic_class);
 
    /* super init */
    pdp_imagebase_init(x);

    /* outlet */
    pdp_base_add_pdp_outlet(x);
    x->x_mask = 0;

    return (void *)x;
}

void *pdp_logic_new_and(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_pdp_inlet(x); 
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_and);

     return (void *)x;
}

void *pdp_logic_new_or(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_pdp_inlet(x); 
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_or);
     return (void *)x;
}

void *pdp_logic_new_xor(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_pdp_inlet(x); 
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_xor);
     return (void *)x;
}

void *pdp_logic_new_not(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_not);
     return (void *)x;
}

void *pdp_logic_new_mask(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_gen_inlet(x, gensym("float"), gensym("mask"));
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_mask);

     x->x_mask = (void *)0xffff;
     return (void *)x;
}

void *pdp_logic_new_depth(void)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_gen_inlet(x, gensym("float"), gensym("depth"));
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_mask);

     x->x_mask = (void *)0xffff;
     return (void *)x;
}

void *pdp_logic_new_softthresh(t_floatarg f)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_gen_inlet(x, gensym("float"), gensym("threshold"));
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_softthresh);
     pdp_logic_set_threshold(x,f);

     return (void *)x;
}


void *pdp_logic_new_hardthresh(t_floatarg f)
{
     t_pdp_logic *x = pdp_logic_new();
     /* init in/out */
     pdp_base_add_gen_inlet(x, gensym("float"), gensym("threshold"));
     pdp_base_set_process_method(x, (t_pdp_method)pdp_logic_process_hardthresh);
     pdp_logic_set_threshold(x,f);

     return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_logic_setup(void)
{


    pdp_logic_class = class_new(gensym("pdp_and"), (t_newmethod)pdp_logic_new_and,
    	(t_method)pdp_logic_free, sizeof(t_pdp_logic), 0, A_DEFFLOAT, A_NULL);

    pdp_imagebase_setup(pdp_logic_class);

    class_addcreator((t_newmethod)pdp_logic_new_or, gensym("pdp_or"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_xor, gensym("pdp_xor"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_not, gensym("pdp_not"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_mask, gensym("pdp_bitmask"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_depth, gensym("pdp_bitdepth"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_softthresh, gensym("pdp_sthresh"), A_NULL);
    class_addcreator((t_newmethod)pdp_logic_new_hardthresh, gensym("pdp_hthresh"), A_NULL);
  
    class_addmethod(pdp_logic_class, (t_method)pdp_logic_set_mask, gensym("mask"), A_FLOAT, A_NULL);
    class_addmethod(pdp_logic_class, (t_method)pdp_logic_set_depth, gensym("depth"), A_FLOAT, A_NULL);
    class_addmethod(pdp_logic_class, (t_method)pdp_logic_set_threshold, gensym("threshold"), A_FLOAT, A_NULL);
}

#ifdef __cplusplus
}
#endif
