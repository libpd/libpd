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


typedef struct pdp_conv_struct
{
    t_pdp_imagebase x_base;


    unsigned int x_nbpasses;
    bool x_horizontal;
    bool x_vertical;

    void *x_convolver_hor;
    void *x_convolver_ver;

} t_pdp_conv;



static void pdp_conv_process(t_pdp_conv *x)
{
    int p = pdp_base_get_packet(x, 0);
    u32 mask = pdp_imagebase_get_chanmask(x);

    if (x->x_vertical){
	pdp_imageproc_conv_setnbpasses(x->x_convolver_ver, x->x_nbpasses);
	pdp_imageproc_dispatch_1buf(&pdp_imageproc_conv_process, x->x_convolver_ver, mask, p);
    }

    if (x->x_horizontal){
	pdp_imageproc_conv_setnbpasses(x->x_convolver_hor, x->x_nbpasses);
	pdp_imageproc_dispatch_1buf(&pdp_imageproc_conv_process, x->x_convolver_hor, mask, p);
    }

}



static void pdp_conv_passes(t_pdp_conv *x, t_floatarg f)
{
    int passes = (int)f;
    passes = passes < 0 ? 0 : passes;
    x->x_nbpasses = passes;

}
static void pdp_conv_hor(t_pdp_conv *x, t_floatarg f)
{
    int hor = (int)f;
    x->x_horizontal = (hor != 0);

}
static void pdp_conv_ver(t_pdp_conv *x, t_floatarg f)
{
    int ver = (int)f;
    x->x_vertical = (ver != 0);
}
static void pdp_conv_free(t_pdp_conv *x)
{
    pdp_imagebase_free(x);
    pdp_imageproc_conv_delete(x->x_convolver_hor);
    pdp_imageproc_conv_delete(x->x_convolver_ver);
}

/* setup hmask */

static void pdp_conv_hleft(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setmin1(x->x_convolver_hor, f);

}
static void pdp_conv_hmiddle(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setzero(x->x_convolver_hor, f);
}
static void pdp_conv_hright(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setplus1(x->x_convolver_hor, f);
}

static void pdp_conv_hmask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_hleft(x, l);
  pdp_conv_hmiddle(x, m);
  pdp_conv_hright(x, r);
}

static void pdp_conv_vtop(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setmin1(x->x_convolver_ver, f);
}
static void pdp_conv_vmiddle(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setzero(x->x_convolver_ver, f);

}
static void pdp_conv_vbottom(t_pdp_conv *x, t_floatarg f)
{
    pdp_imageproc_conv_setplus1(x->x_convolver_ver, f);
}

static void pdp_conv_vmask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_vtop(x, l);
  pdp_conv_vmiddle(x, m);
  pdp_conv_vbottom(x, r);
}


static void pdp_conv_mask(t_pdp_conv *x, t_floatarg l, t_floatarg m, t_floatarg r)
{
  pdp_conv_hmask(x, l, m, r);
  pdp_conv_vmask(x, l, m, r);
}

t_class *pdp_conv_class;



void *pdp_conv_new(void)
{
    t_pdp_conv *x = (t_pdp_conv *)pd_new(pdp_conv_class);

    /* super init */
    pdp_imagebase_init(x);

    /* inlets & outlets */
    pdp_base_add_gen_inlet(x, gensym("float"), gensym("passes"));
    pdp_base_add_pdp_outlet(x);

    /* register */
    pdp_base_set_process_method(x, (t_pdp_method)pdp_conv_process);

    x->x_nbpasses = 1;
    x->x_horizontal = true;
    x->x_vertical = true;

    x->x_convolver_hor = pdp_imageproc_conv_new();
    x->x_convolver_ver = pdp_imageproc_conv_new();

    pdp_imageproc_conv_setbordercolor(x->x_convolver_hor, 0);
    pdp_imageproc_conv_setbordercolor(x->x_convolver_ver, 0);

    pdp_imageproc_conv_setorientation(x->x_convolver_hor, PDP_IMAGEPROC_CONV_HORIZONTAL);
    pdp_imageproc_conv_setorientation(x->x_convolver_ver, PDP_IMAGEPROC_CONV_VERTICAL);
 
    pdp_conv_mask(x, .25,.5,.25);

    return (void *)x;
}


#ifdef __cplusplus
extern "C"
{
#endif


void pdp_conv_setup(void)
{


    pdp_conv_class = class_new(gensym("pdp_conv"), (t_newmethod)pdp_conv_new,
    	(t_method)pdp_conv_free, sizeof(t_pdp_conv), 0, A_NULL);

    pdp_imagebase_setup(pdp_conv_class);

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_passes, gensym("passes"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hor, gensym("hor"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_ver, gensym("ver"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hleft, gensym("hleft"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hmiddle, gensym("hmiddle"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hright, gensym("hright"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vtop, gensym("vtop"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vmiddle, gensym("vmiddle"),  A_DEFFLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vbottom, gensym("vbottom"),  A_DEFFLOAT, A_NULL);   

    class_addmethod(pdp_conv_class, (t_method)pdp_conv_vmask, gensym("vmask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_hmask, gensym("hmask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   
    class_addmethod(pdp_conv_class, (t_method)pdp_conv_mask, gensym("mask"),  A_FLOAT, A_FLOAT, A_FLOAT, A_NULL);   

    

}

#ifdef __cplusplus
}
#endif
