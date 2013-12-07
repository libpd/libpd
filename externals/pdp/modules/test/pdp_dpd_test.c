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
#include "pdp_dpd_base.h"

typedef struct pdp_dpd_test_struct
{
    t_pdp_dpd_base x_base;

} t_pdp_dpd_test;



/* outlet methods */
static void pdp_dpd_test_1(t_pdp_dpd_test *x){post("%x: one", x);}
static void pdp_dpd_test_2(t_pdp_dpd_test *x){post("%x: two", x);}
static void pdp_dpd_test_3(t_pdp_dpd_test *x){post("%x: three", x);}
static void pdp_dpd_test_cleanup(t_pdp_dpd_test *x){post("%x: cleanup", x);}
static void pdp_dpd_test_inspect(t_pdp_dpd_test *x){post("%x: inspect", x);}



static void pdp_dpd_test_bang(t_pdp_dpd_test *x)
{
    /* store a dummy packet */
    pdp_dpd_base_set_context_packet(x, pdp_packet_new(PDP_IMAGE, 4096));

    /* bang base */
    pdp_dpd_base_bang(x);
}


static void pdp_dpd_test_free(t_pdp_dpd_test *x)
{
    pdp_dpd_base_free(x);

}

t_class *pdp_dpd_test_class;


void *pdp_dpd_test_new(void)
{
    /* allocate */
    t_pdp_dpd_test *x = (t_pdp_dpd_test *)pd_new(pdp_dpd_test_class);

    /* init super: this is mandatory */
    pdp_dpd_base_init(x);

    /* set the dpd processing methods & outlets */
    pdp_dpd_base_add_outlet(x, (t_pdp_method)pdp_dpd_test_1);
    pdp_dpd_base_add_outlet(x, (t_pdp_method)pdp_dpd_test_2);
    pdp_dpd_base_add_outlet(x, (t_pdp_method)pdp_dpd_test_3);

    pdp_dpd_base_add_cleanup(x, (t_pdp_method)pdp_dpd_test_cleanup);
    pdp_dpd_base_add_inspector(x, (t_pdp_method)pdp_dpd_test_inspect);

    return (void *)x;
}



#ifdef __cplusplus
extern "C"
{
#endif


void pdp_dpd_test_setup(void)
{
    /* create a standard pd class */
    pdp_dpd_test_class = class_new(gensym("pdp_dpd_test"), (t_newmethod)pdp_dpd_test_new,
   	(t_method)pdp_dpd_test_free, sizeof(t_pdp_dpd_test), 0, A_NULL);

    /* inherit pdp base class methods */
    pdp_dpd_base_setup(pdp_dpd_test_class);

    /* add bang method */
    class_addbang(pdp_dpd_test_class, pdp_dpd_test_bang);
}

#ifdef __cplusplus
}
#endif
