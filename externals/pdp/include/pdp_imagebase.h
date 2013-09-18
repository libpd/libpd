/*
 *   Pure Data Packet image processor base class header file.
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
  This file contains the specification of the pdp base class. It is derived
  from t_object, the basic pd object (like any other pd extern). Have a look
  at pdp_add, pdp_gain and pdp_noise to see how to use this.

*/


#include "pdp_base.h"


typedef struct
{
    t_pdp_base x_obj;
    u32 b_channel_mask;     // channel mask

} t_pdp_imagebase;



/* setup base class. call this in your derived class setup method */
void pdp_imagebase_setup(t_class *c);


/* base class constructor/destructor. call this in your base class constructor/destructor */
void pdp_imagebase_init(void *x);
void pdp_imagebase_free(void *x);

/* getters for image base class data */
u32 pdp_imagebase_get_chanmask(void *x);
