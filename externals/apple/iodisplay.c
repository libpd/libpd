/* --------------------------------------------------------------------------*/
/*                                                                           */
/* control the iodisplay of the display on Apple Mac OS X                   */
/* Written by Hans-Christoph Steiner <hans@eds.org>                         */
/*                                                                           */
/* Copyright (c) 2008 Free Software Foundation                               */
/*                                                                           */
/* This program is free software; you can redistribute it and/or             */
/* modify it under the terms of the GNU General Public License               */
/* as published by the Free Software Foundation; either version 2            */
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

#include <IOKit/graphics/IOGraphicsLib.h>
#include <ApplicationServices/ApplicationServices.h> 
#include <m_pd.h>

#define DEBUG(x)
//#define DEBUG(x) x 

/*------------------------------------------------------------------------------
 *  CLASS DEF
 */

static t_class *iodisplay_class;

typedef struct _iodisplay {
    t_object            x_obj;
    t_float             parameter_value;
    io_service_t        io_service;
    CGDirectDisplayID   target_display;
    t_symbol*           parameter;
} t_iodisplay;

/*------------------------------------------------------------------------------
 * IMPLEMENTATION                    
 */

static void iodisplay_output(t_iodisplay* x)
{
	DEBUG(post("iodisplay_output"););
    CFStringRef cfs_parameter;
    CGDisplayErr err;
    t_atom output_atom;

    cfs_parameter = CFStringCreateWithCString(kCFAllocatorDefault,
                                              x->parameter->s_name, 
                                              kCFStringEncodingASCII);
     err = IODisplayGetFloatParameter(x->io_service, kNilOptions, 
                                     cfs_parameter, &(x->parameter_value));
	if (err != kIOReturnSuccess)	
        pd_error(x,"[iodisplay]: couldn't get %s value", x->parameter->s_name);

    SETFLOAT(&output_atom, x->parameter_value);
    outlet_anything(x->x_obj.ob_outlet, x->parameter, 1, &output_atom);
}


static void iodisplay_float(t_iodisplay* x, t_float f)
{
	DEBUG(post("iodisplay_float"););
    CFStringRef cfs_parameter;
    CGDisplayErr err;  
    
	if (f < 0.) x->parameter_value = 0.;
    else if (f > 1.) x->parameter_value = 1.;
    else x->parameter_value = f;
    
    cfs_parameter = CFStringCreateWithCString(kCFAllocatorDefault,
                                              x->parameter->s_name, 
                                              kCFStringEncodingASCII);
	err = IODisplaySetFloatParameter(x->io_service, kNilOptions, 
                                     cfs_parameter, x->parameter_value);  
	if (err != kIOReturnSuccess)
        pd_error(x,"[iodisplay]: couldn't set %s", x->parameter->s_name);
}


static void iodisplay_free(t_iodisplay* x)
{
	DEBUG(post("iodisplay_free"););
}


static void *iodisplay_new(t_symbol *s) 
{
	DEBUG(post("iodisplay_new"););
	t_iodisplay *x = (t_iodisplay *)pd_new(iodisplay_class);

    x->target_display = CGMainDisplayID();
    x->io_service = CGDisplayIOServicePort(x->target_display);
    x->parameter = s;

    symbolinlet_new(&x->x_obj, &x->parameter);
	outlet_new(&x->x_obj, &s_anything);
	
	return (x);
}

void iodisplay_setup(void) 
{
	iodisplay_class = class_new(gensym("iodisplay"), 
                              (t_newmethod)iodisplay_new,
                              (t_method)iodisplay_free,
                              sizeof(t_iodisplay), 
                              CLASS_DEFAULT, 
                              A_DEFSYMBOL, 0);

	/* add inlet datatype methods */
	class_addbang(iodisplay_class,(t_method) iodisplay_output);
	class_addfloat(iodisplay_class,(t_method) iodisplay_float);
}
