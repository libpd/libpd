/* --------------------------------------------------------------------------*/
/*                                                                           */
/* get info from the multitouch trackpad on Apple Mac OS X                   */
/* based on 'fingerpinger'                                                  */
/*                                                                           */
/* Copyright (c) 2009 Hans-Christoph Steiner                                 */
/* Copyright (c) 2009 Michael & Max Egger                                    */
/* Copyright (c) 2008 Steike                                                 */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 3            */
/* of the License, or (at your option) any later version.                    */
/*                                                                           */
/* See file LICENSE for further informations on licensing terms.             */
/*                                                                           */
/* This program is distributed in the hope that it will be useful,           */
/* but WITHOUT ANY WARRANTY; without even the implied warranty of            */
/* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             */
/* GNU General Public License for more details.                              */
/*                                                                           */
/* You should have received a copy of the GNU General Public License         */
/* along with this program; if not, write to the Free Software Foundation,   */
/* Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA */
/*                                                                           */
/* --------------------------------------------------------------------------*/

#include <mach/mach.h> 
#include <IOKit/IOKitLib.h> 
#include <CoreFoundation/CoreFoundation.h> 
#include <math.h>
#include <unistd.h>
#include "MultitouchSupport.h"
#include <m_pd.h>

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *multitouch_class;

typedef struct _multitouch {
    t_object            x_obj;
    t_outlet*           data_outlet;
    t_outlet*           status_outlet;
} t_multitouch;

static MTDeviceRef dev;      /* reference to the trackpad */
static int fingerc;  /* current count of Fingers */
static Finger fingerv[32];  /* current list of Fingers */
static int multitouch_instances = 0; /* set when one instance is polling so others don't */

/*------------------------------------------------------------------------------
 * CALLBACK TO GET DATA
 */

static int callback(int device, Finger *data, int nFingers, double timestamp, int frame) 
{
	DEBUG(post("callback"););
    memcpy(fingerv, data, nFingers * sizeof(Finger));
    fingerc = nFingers;
    return 0;
}

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void multitouch_output(t_multitouch* x)
{
	DEBUG(post("multitouch_output"););
	int i;
	t_atom output_list[12];
	for (i=0; i < fingerc; i++) {
		Finger *f = &fingerv[i];
		SETFLOAT(output_list,i);
		SETFLOAT(output_list + 1 , f->frame);
		SETFLOAT(output_list + 2 , f->angle);
		SETFLOAT(output_list + 3 , f->majorAxis);
		SETFLOAT(output_list + 4 , f->minorAxis);
		SETFLOAT(output_list + 5 , f->normalized.pos.x);
		SETFLOAT(output_list + 6 , f->normalized.pos.y);
		SETFLOAT(output_list + 7 , f->normalized.vel.x);
		SETFLOAT(output_list + 8 , f->normalized.vel.y);
		SETFLOAT(output_list + 9 , f->identifier);
		SETFLOAT(output_list + 10 , f->state);
		SETFLOAT(output_list + 11 , f->size);
        outlet_list(x->data_outlet, &s_, 12, output_list);
    }
}

static void multitouch_info(t_multitouch* x)
{
    t_atom output_atom;
    SETFLOAT(&output_atom, fingerc);
    outlet_anything(x->status_outlet, gensym("fingers"), 1, &output_atom);
}

static void multitouch_free(t_multitouch* x)
{
	DEBUG(post("multitouch_free"););
    multitouch_instances--;
    /* if I am the last instance, clean up the callback stuff */
    if (multitouch_instances == 0) {
        MTDeviceStop(dev);
        MTUnregisterContactFrameCallback(dev, callback);
        MTDeviceRelease(dev);
        dev = NULL;
    }
}

static void *multitouch_new(void) 
{
	DEBUG(post("multitouch_new"););
	t_multitouch *x = (t_multitouch *)pd_new(multitouch_class);

    multitouch_instances++;
    /* if I am the first instance to poll, then set the callback up */
    if (multitouch_instances == 1) {
        dev = MTDeviceCreateDefault();
        MTRegisterContactFrameCallback(dev, callback);
        MTDeviceStart(dev, 0);
    }

    x->data_outlet = outlet_new(&x->x_obj, &s_list);
	x->status_outlet = outlet_new(&x->x_obj, &s_anything);

	return (x);
}

void multitouch_setup(void) 
{
	multitouch_class = class_new(gensym("multitouch"), 
                                        (t_newmethod)multitouch_new,
                                        (t_method)multitouch_free,
                                        sizeof(t_multitouch), 
                                        CLASS_DEFAULT,
                                        0);
	/* add inlet datatype methods */
	class_addbang(multitouch_class,(t_method) multitouch_output);
	class_addmethod(multitouch_class,(t_method) multitouch_info, gensym("info"), 0);
}
