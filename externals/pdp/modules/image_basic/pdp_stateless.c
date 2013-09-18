/*
 *   Pure Data Packet module. Some stateless image operations.
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

typedef struct pdp_stateless_struct
{
    t_pdp_imagebase x_base;

} t_pdp_stateless;



static void pdp_stateless_process_abs(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_abs_process, 0, mask, p0);
}

static void pdp_stateless_process_hardthresh(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_hardthresh_process, 0, mask, p0);
}

static void pdp_stateless_process_zthresh(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_zthresh_process, 0, mask, p0);
}

static void pdp_stateless_process_positive(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_ispositive_process, 0, mask, p0);
}

static void pdp_stateless_process_sign(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_sign_process, 0, mask, p0);
}

static void pdp_stateless_process_flip_tb(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_flip_tb_process, 0, mask, p0);
}

static void pdp_stateless_process_flip_lr(t_pdp_stateless *x)
{
    int p0 = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);
    pdp_imageproc_dispatch_1buf(&pdp_imageproc_flip_lr_process, 0, mask, p0);
}

static void pdp_stateless_free(t_pdp_stateless *x)
{
    /* remove process method from queue before deleting data */
    pdp_imagebase_free(x);
}

t_class *pdp_stateless_class;


/* common new method */
void *pdp_stateless_new(void)
{
    t_pdp_stateless *x = (t_pdp_stateless *)pd_new(pdp_stateless_class);
 
    /* super init */
    pdp_imagebase_init(x);

    /* outlet */
    pdp_base_add_pdp_outlet(x);

    return (void *)x;
}

void *pdp_stateless_new_abs(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_abs);
     return (void *)x;
}

void *pdp_stateless_new_zthresh(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_zthresh);
     return (void *)x;
}

void *pdp_stateless_new_positive(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_positive);
     return (void *)x;
}

void *pdp_stateless_new_sign(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_sign);
     return (void *)x;
}

void *pdp_stateless_new_flip_tb(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_flip_tb);
     return (void *)x;
}


void *pdp_stateless_new_flip_lr(void)
{
     t_pdp_stateless *x = pdp_stateless_new();
     /* init in/out */
     pdp_base_set_process_method(x, (t_pdp_method)pdp_stateless_process_flip_lr);
     return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_stateless_setup(void)
{


    pdp_stateless_class = class_new(gensym("pdp_abs"), (t_newmethod)pdp_stateless_new_abs,
    	(t_method)pdp_stateless_free, sizeof(t_pdp_stateless), 0, A_NULL);

    pdp_imagebase_setup(pdp_stateless_class);

    class_addcreator((t_newmethod)pdp_stateless_new_zthresh, gensym("pdp_zthresh"), A_NULL);
    class_addcreator((t_newmethod)pdp_stateless_new_positive, gensym("pdp_positive"), A_NULL);
    class_addcreator((t_newmethod)pdp_stateless_new_sign, gensym("pdp_sign"), A_NULL);
    class_addcreator((t_newmethod)pdp_stateless_new_flip_tb, gensym("pdp_flip_tb"), A_NULL);
    class_addcreator((t_newmethod)pdp_stateless_new_flip_lr, gensym("pdp_flip_lr"), A_NULL);

    /* future extensions */
    //class_addcreator((t_newmethod)pdp_stateless_new_garble, gensym("pdp_garble"), A_NULL);
  

}

#ifdef __cplusplus
}
#endif
