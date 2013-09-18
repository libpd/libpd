/*
 *   Pure Data Packet image processor base class implementation.
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


/*

  This file contains the pdp image base class object.
*/

#include "pdp_imagebase.h"
#include <stdarg.h>


static void pdp_imagebase_chanmask(t_pdp_base *b, t_floatarg f)
{
    int i = (int)f;
    if (i < 0) i = -1;
    b->b_channel_mask = i;
}

void pdp_imagebase_setup(t_class *c)
{
    /* parent class setup */
    pdp_base_setup(c);

     /* add pdp base class methods */
    class_addmethod(c, (t_method)pdp_imagebase_chanmask, gensym("chanmask"), A_FLOAT, A_NULL);

}

/* pdp base instance constructor */
void pdp_imagebase_init(void *x)
{
    int i;
    t_pdp_imagebase *b = (t_pdp_imagebase *)x;

    /* init super */
    pdp_base_init(x);

    /* convert all active incoming packet types to image */
    pdp_base_set_type_template(x, 0, pdp_gensym("image/*/*"));

    /* default chanmask == all */
    b->b_channel_mask = -1;

}

/* base instance destructor */
void pdp_imagebase_free(void *x)
{
    /* free super */
    pdp_base_free(x);

}

/* chanmask getter */
u32 pdp_imagebase_get_chanmask(void *x)
{
    t_pdp_base *b = (t_pdp_base *)x;
    return b->b_channel_mask;
}

